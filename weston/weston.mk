if ENABLE_WESTON
westonplugin_LTLIBRARIES = weston-notification-area.la
endif

weston_notification_area_la_SOURCES = \
	%D%/src/notification-area.c \
	$(null)

nodist_weston_notification_area_la_SOURCES = \
	src/notification-area-unstable-v2-protocol.c \
	src/notification-area-unstable-v2-server-protocol.h \
	$(null)

weston_notification_area_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(WESTON_CFLAGS) \
	$(WAYLAND_CFLAGS) \
	$(null)

weston_notification_area_la_LIBADD = \
	$(WESTON_LIBS) \
	$(WAYLAND_LIBS) \
	$(null)

weston-notification-area.la %D%/src/weston_notification_area_la-notification-area.lo: src/notification-area-unstable-v2-server-protocol.h
