if ENABLE_WLC
AM_CFLAGS += -I$(srcdir)/%D%/include
lib_LTLIBRARIES += libwlc-notification-area.la
pkginclude_HEADERS += %D%/include/wlc-notification-area.h
pkgconfig_DATA += %D%/pkgconfig/wlc-notification-area.pc
endif

libwlc_notification_area_la_SOURCES = \
	%D%/include/wlc-notification-area.h \
	%D%/src/wlc-notification-area.c \
	$(null)

nodist_libwlc_notification_area_la_SOURCES = \
	src/notification-area-unstable-v2-protocol.c \
	src/notification-area-unstable-v2-server-protocol.h \
	$(null)

libwlc_notification_area_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(WLC_CFLAGS) \
	$(WAYLAND_CFLAGS) \
	$(null)

libwlc_notification_area_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-static \
	$(null)

libwlc_notification_area_la_LIBADD = \
	$(WLC_LIBS) \
	$(WAYLAND_LIBS) \
	$(null)

libwlc-notification-area.la %D%/src/libwlc_notification_area_la-wlc-notification-area.lo: src/notification-area-unstable-v2-server-protocol.h
