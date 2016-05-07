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

#include <wayland-util.h>
#include <orbment/plugin.h>
#include <wlc-notification-area.h>

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
