/*
 * Copyright © 2013-2016 Quentin “Sardem FF7” Glidic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <wayland-util.h>
#include <wlc/wlc.h>
#include <wlc/wlc-wayland.h>
#include "notification-area-unstable-v1-server-protocol.h"

#include <wayland-wall-wlc.h>

struct ww_wlc_notification_area {
    struct wl_global *global;
    struct wl_resource *binding;
    struct wl_list notifications;
    wlc_handle output;
};

struct ww_wlc_notification_area_notification {
    struct ww_wlc_notification_area *na;
    struct wl_list link;
    wlc_handle view;
};

static void
_ww_wlc_notification_area_request_destroy(struct wl_client *client, struct wl_resource *resource)
{
    (void) client;
    wl_resource_destroy(resource);
}

static void
_ww_wlc_notification_area_notification_set_output(struct ww_wlc_notification_area_notification *notification, wlc_handle output)
{
    if ( output == 0 )
    {
        wlc_view_set_mask(notification->view, 0);
        return;
    }

    wlc_view_set_output(notification->view, output);
    wlc_view_set_mask(notification->view, wlc_output_get_mask(output));
    wlc_view_bring_to_front(notification->view);
}

static void
_ww_wlc_notification_area_notification_request_move(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y)
{
    (void) client;
    struct ww_wlc_notification_area_notification *notification = wl_resource_get_user_data(resource);
    struct wlc_geometry geometry;

    geometry = *wlc_view_get_geometry(notification->view);
    geometry.origin.x = x;
    geometry.origin.y = y;
    wlc_view_set_geometry(notification->view, WLC_RESIZE_EDGE_NONE, &geometry);
    if ( wlc_view_get_mask(notification->view) == 0 )
        _ww_wlc_notification_area_notification_set_output(notification, notification->na->output);
}

static const struct zww_notification_v1_interface _ww_wlc_notification_area_notification_implementation = {
    .destroy = _ww_wlc_notification_area_request_destroy,
    .move = _ww_wlc_notification_area_notification_request_move,
};

static void
_ww_wlc_notification_area_destroy(struct wl_client *client, struct wl_resource *resource)
{
    (void) client;
    wl_resource_destroy(resource);
}

static void
_ww_wlc_notification_area_create_notification(struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *surface_resource)
{
    struct ww_wlc_notification_area *self = wl_resource_get_user_data(resource);

    struct ww_wlc_notification_area_notification *notification;
    notification = calloc(1, sizeof(struct ww_wlc_notification_area_notification));
    if ( notification == NULL )
    {
        wl_resource_post_no_memory(surface_resource);
        return;
    }
    notification->na = self;
    wl_list_insert(&self->notifications, &notification->link);

    notification->view = wlc_view_from_surface(wlc_resource_from_wl_surface_resource(surface_resource), client, &zww_notification_v1_interface, &_ww_wlc_notification_area_notification_implementation, 1, id, notification);
    if ( notification->view == 0 )
        return;
    wlc_view_set_type(notification->view, WLC_BIT_OVERRIDE_REDIRECT, true);
    wlc_view_set_type(notification->view, WLC_BIT_UNMANAGED, true);
    wlc_view_set_mask(notification->view, 0);
}

static const struct zww_notification_area_v1_interface _ww_wlc_notification_area_implementation = {
    .destroy = _ww_wlc_notification_area_destroy,
    .create_notification = _ww_wlc_notification_area_create_notification,
};

static void
_ww_wlc_notification_area_unbind(struct wl_resource *resource)
{
    struct ww_wlc_notification_area *self = resource->data;

    self->binding = NULL;
}

static void
_ww_wlc_notification_area_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
    struct ww_wlc_notification_area *self = data;
    struct wl_resource *resource;

    resource = wl_resource_create(client, &zww_notification_area_v1_interface, version, id);
    wl_resource_set_implementation(resource, &_ww_wlc_notification_area_implementation, self, _ww_wlc_notification_area_unbind);

    if ( self->binding != NULL )
    {
        wl_resource_post_error(resource, ZWW_NOTIFICATION_AREA_V1_ERROR_BOUND, "interface object already bound");
        wl_resource_destroy(resource);
        return;
    }

    self->binding = resource;

    const struct wlc_size *size;
    if ( self->output == 0 )
        size = &wlc_size_zero;
    else
        size = wlc_output_get_resolution(self->output);
    zww_notification_area_v1_send_geometry(self->binding, size->w, size->h, 1);
}

struct ww_wlc_notification_area *
ww_wlc_notification_area_init(void)
{
    struct ww_wlc_notification_area *self;

    self = calloc(1, sizeof(struct ww_wlc_notification_area));

    wl_list_init(&self->notifications);

    if ( ( self->global = wl_global_create(wlc_get_wl_display(), &zww_notification_area_v1_interface, 1, self, _ww_wlc_notification_area_bind) ) == NULL )
    {
        free(self);
        return NULL;
    }
    wl_global_destroy(self->global);
    if ( ( self->global = wl_global_create(wlc_get_wl_display(), &zww_notification_area_v1_interface, 1, self, _ww_wlc_notification_area_bind) ) == NULL )
    {
        free(self);
        return NULL;
    }

    return self;
}

static void
_ww_wlc_notification_area_view_destroy_internal(struct ww_wlc_notification_area_notification *self)
{
    //wlc_view_close(notification->view);

    wl_list_remove(&self->link);

    free(self);
}

void
ww_wlc_notification_area_uninit(struct ww_wlc_notification_area *self)
{
    struct ww_wlc_notification_area_notification *notification, *tmp;
    wl_list_for_each_safe(notification, tmp, &self->notifications, link)
        _ww_wlc_notification_area_view_destroy_internal(notification);
    wl_global_destroy(self->global);

    free(self);
}

wlc_handle
ww_wlc_notification_area_get_output(struct ww_wlc_notification_area *self)
{
    return self->output;
}

void
ww_wlc_notification_area_set_output(struct ww_wlc_notification_area *self, wlc_handle output)
{
    if ( self->binding != NULL )
    {
        const struct wlc_size *size;
        if ( output == 0 )
            size = &wlc_size_zero;
        else
            size = wlc_output_get_resolution(output);
        zww_notification_area_v1_send_geometry(self->binding, size->w, size->h, 1);
    }

    if ( self->output == output )
        return;
    self->output = output;

    struct ww_wlc_notification_area_notification *notification;
    wl_list_for_each(notification, &self->notifications, link)
    {
        if ( wlc_view_get_mask(notification->view) == 0 )
            continue;

        _ww_wlc_notification_area_notification_set_output(notification, output);
    }
}

void
ww_wlc_notification_area_view_destroy(struct ww_wlc_notification_area *self, wlc_handle view)
{
    struct ww_wlc_notification_area_notification *notification, *tmp;
    wl_list_for_each_safe(notification, tmp, &self->notifications, link)
    {
        if ( notification->view != view )
            continue;

        _ww_wlc_notification_area_view_destroy_internal(notification);
        return;
    }
}
