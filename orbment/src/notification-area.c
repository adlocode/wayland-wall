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

#include <wayland-util.h>
#include <orbment/plugin.h>
#include "wlc-notification-area.h"

static struct {
    plugin_h self;
    struct wlc_notification_area *na;
} plugin;

static bool
_wlc_notification_area_output_created(wlc_handle output)
{
    if ( wlc_notification_area_get_output(plugin.na) == 0 )
        wlc_notification_area_set_output(plugin.na, output);

    return true;
}

static void
_wlc_notification_area_output_destroyed(wlc_handle output)
{
    if ( wlc_notification_area_get_output(plugin.na) != output )
        return;

    const wlc_handle *outputs;
    size_t size;
    outputs = wlc_get_outputs(&size);

    wlc_notification_area_set_output(plugin.na, ( size > 0 ) ? outputs[0] : 0);
}

static void
_wlc_notification_area_output_resolution(wlc_handle output, const struct wlc_size *from, const struct wlc_size *to)
{
    (void) from;
    (void) to;

    if ( wlc_notification_area_get_output(plugin.na) != output )
        return;

    wlc_notification_area_set_output(plugin.na, output);
}

static void
_wlc_notification_area_output_focus(wlc_handle output, bool focused)
{
    if ( ! focused )
        return;

    if ( ! false )
        return;

    wlc_notification_area_set_output(plugin.na, output);
}

static void
_wlc_notification_area_view_destroyed(wlc_handle view)
{
    wlc_notification_area_view_destroy(plugin.na, view);
}

#pragma GCC diagnostic ignored "-Wmissing-prototypes"

WNA_EXPORT
PCONST void
plugin_deinit(plugin_h self)
{
    (void) self;

    wlc_notification_area_uninit(plugin.na);
}

WNA_EXPORT
bool
plugin_init(plugin_h p)
{
    plugin.self = p;

    plugin_h orbment;
    if ( ( orbment = import_plugin(plugin.self, "orbment") ) == 0 )
        return false;

    bool (*add_hook)(plugin_h, const char *name, const struct function*);
    if ( ( add_hook = import_method(plugin.self, orbment, "add_hook", "b(h,c[],fun)|1") ) == NULL)
        return false;

    if ( ( plugin.na = wlc_notification_area_init() ) == NULL )
        return false;

    if ( ! add_hook(plugin.self, "output.created", FUN(_wlc_notification_area_output_created, "b(h)|1")) )
        return false;

    if ( ! add_hook(plugin.self, "output.destroyed", FUN(_wlc_notification_area_output_destroyed, "v(h)|1")) )
        return false;

    if ( ! add_hook(plugin.self, "output.resolution", FUN(_wlc_notification_area_output_resolution, "v(h,*,*)|1")) )
        return false;

    if ( ! add_hook(plugin.self, "output.focus", FUN(_wlc_notification_area_output_focus, "v(h,b)|1")) )
        return false;

    if ( ! add_hook(plugin.self, "view.destroyed", FUN(_wlc_notification_area_view_destroyed, "v(h)|1")) )
        return false;

    return true;
}

WNA_EXPORT
PCONST const struct plugin_info*
plugin_register(void)
{
    static const struct plugin_info info = {
        .name = "notification-area",
        .description = "Notification area protocol implementation.",
        .version = PACKAGE_VERSION,
    };

    return &info;
}
