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

#ifndef _WLC_NOTIFICATION_DAEMON_H_
#define _WLC_NOTIFICATION_DAEMON_H_

#include <wlc/wlc.h>

struct wlc_notification_area;

struct wlc_notification_area *wlc_notification_area_init(void);
void wlc_notification_area_uninit(struct wlc_notification_area *na);
WLC_PURE wlc_handle wlc_notification_area_get_output(struct wlc_notification_area *na);
void wlc_notification_area_set_output(struct wlc_notification_area *na, wlc_handle output);
void wlc_notification_area_view_destroy(struct wlc_notification_area *na, wlc_handle view);

#endif /* _WLC_NOTIFICATION_DAEMON_H_ */
