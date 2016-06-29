if ENABLE_WLC
AM_CFLAGS += -I$(srcdir)/%D%/include
lib_LTLIBRARIES += libwayland-wall-wlc.la
pkginclude_HEADERS += %D%/include/wayland-wall-wlc.h
pkgconfig_DATA += %D%/pkgconfig/wayland-wall-wlc.pc
endif

libwayland_wall_wlc_la_SOURCES = \
	%D%/src/wlc-notification-area.c \
	$(null)

nodist_libwayland_wall_wlc_la_SOURCES = \
	src/notification-area-unstable-v1-protocol.c \
	src/notification-area-unstable-v1-server-protocol.h \
	$(null)

libwayland_wall_wlc_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(WLC_CFLAGS) \
	$(WAYLAND_CFLAGS) \
	$(null)

libwayland_wall_wlc_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-static \
	$(null)

libwayland_wall_wlc_la_LIBADD = \
	$(WLC_LIBS) \
	$(WAYLAND_LIBS) \
	$(null)

libwayland-wall-wlc.la %D%/src/libwayland_wall_wlc_la-wlc-notification-area.lo: src/notification-area-unstable-v1-server-protocol.h
