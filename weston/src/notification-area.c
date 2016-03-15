/*
 * Copyright © 2013-2016 Quentin “Sardem FF7” Glidic
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <assert.h>
#include <signal.h>
#include <math.h>
#include <sys/types.h>

#include <wayland-server.h>
#include <weston/compositor.h>
#include "notification-area-unstable-v1-server-protocol.h"

struct weston_notification_area {
    struct weston_compositor *compositor;
    struct wl_resource *binding;
    struct weston_layer layer;
    struct weston_output *output;
    pixman_rectangle32_t workarea;
};

struct weston_notification_area_notification {
    struct wl_resource *resource;
    struct weston_notification_area *na;
    struct weston_surface *surface;
    struct weston_view *view;
};

static void
_weston_notification_area_request_destroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static struct weston_output *
_weston_notification_area_get_default_output(struct weston_notification_area *na)
{
    return wl_container_of(na->compositor->output_list.next, na->output, link);
}

static void
_weston_notification_area_notification_request_move(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y)
{
    struct weston_notification_area_notification *self = wl_resource_get_user_data(resource);

    x += self->na->workarea.x;
    y += self->na->workarea.y;

    weston_view_set_position(self->view, x, y);

    weston_view_geometry_dirty(self->view);
    weston_view_update_transform(self->view);
    weston_surface_damage(self->surface);
}

static const struct zwna_notification_v1_interface weston_notification_area_notification_implementation = {
    .destroy = _weston_notification_area_request_destroy,
    .move = _weston_notification_area_notification_request_move,
};

static void
_weston_notification_area_notification_destroy(struct wl_resource *resource)
{
    struct weston_notification_area_notification *self = wl_resource_get_user_data(resource);

    self->resource = NULL;
}

static void
_weston_notification_area_destroy(struct wl_client *client, struct wl_resource *resource)
{
    struct weston_notification_area_notification *self = wl_resource_get_user_data(resource);

    weston_view_destroy(self->view);

    free(self);
}

static void
_weston_notification_area_create_notification(struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *surface_resource)
{
    struct weston_notification_area *na = wl_resource_get_user_data(resource);
    struct weston_surface *surface = wl_resource_get_user_data(surface_resource);

    if (weston_surface_set_role(surface, "wp_notification", resource, ZWNA_NOTIFICATION_AREA_V1_ERROR_ROLE) < 0)
        return;

    struct weston_notification_area_notification *notification;
    notification = zalloc(sizeof(struct weston_notification_area_notification));
    if ( notification == NULL ) {
        wl_resource_post_no_memory(surface_resource);
        return;
    }
    notification->na = na;

    notification->resource = wl_resource_create(client, &zwna_notification_v1_interface, 1, id);
    notification->surface = surface;
    notification->view = weston_view_create(notification->surface);
    notification->view->output = na->output;
    weston_layer_entry_insert(&na->layer.view_list, &notification->view->layer_link);


    wl_resource_set_implementation(notification->resource, &weston_notification_area_notification_implementation, notification, _weston_notification_area_notification_destroy);
}

static const struct zwna_notification_area_v1_interface weston_notification_area_implementation = {
    .destroy = _weston_notification_area_destroy,
    .create_notification = _weston_notification_area_create_notification,
};

static void
_weston_notification_area_unbind(struct wl_resource *resource)
{
    struct weston_notification_area *na = wl_resource_get_user_data(resource);

    na->binding = NULL;
}

static void
_weston_notification_area_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct weston_notification_area *na = data;
    struct wl_resource *resource;

    resource = wl_resource_create(client, &zwna_notification_area_v1_interface, version, id);
    wl_resource_set_implementation(resource, &weston_notification_area_implementation, na, _weston_notification_area_unbind);

    if ( na->binding != NULL )
    {
        wl_resource_post_error(resource, ZWNA_NOTIFICATION_AREA_V1_ERROR_BOUND, "interface object already bound");
        wl_resource_destroy(resource);
        return;
    }

    na->binding = resource;

    zwna_notification_area_v1_send_geometry(na->binding, na->workarea.width, na->workarea.height);
}

static void
_weston_notification_area_set_output(struct weston_notification_area *na, struct weston_output *output)
{
    na->output = output;
#ifdef WESTON_HAS_WORKAREA_SUPPORT
    na->compositor->shell_interface.get_output_work_area(na->compositor->shell_interface.shell, na->output, &na->workarea);
#else /* ! WESTON_HAS_WORKAREA_SUPPORT */
    na->workarea.x = na->output->x;
    na->workarea.y = na->output->y;
    na->workarea.width = na->output->width;
    na->workarea.height = na->output->height;
#endif /* ! WESTON_HAS_WORKAREA_SUPPORT */
}

WNA_EXPORT int
module_init(struct weston_compositor *compositor, int *argc, char *argv[])
{
    struct weston_notification_area *na;

    na = zalloc(sizeof(struct weston_notification_area));
    if ( na == NULL )
        return -1;

    na->compositor = compositor;

    if ( wl_global_create(na->compositor->wl_display, &zwna_notification_area_v1_interface, 1, na, _weston_notification_area_bind) == NULL)
        return -1;

    weston_layer_init(&na->layer, &na->compositor->cursor_layer.link);

    _weston_notification_area_set_output(na, _weston_notification_area_get_default_output(na));

    return 0;
}
