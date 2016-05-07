if ENABLE_ORBMENT
orbmentplugin_LTLIBRARIES = orbment-plugin-notification-area.la
endif

orbment_plugin_notification_area_la_SOURCES = \
	%D%/src/notification-area.c \
	%D%/src/wlc-notification-area.h \
	%D%/src/wlc-notification-area.c \
	$(null)

nodist_orbment_plugin_notification_area_la_SOURCES = \
	src/notification-area-unstable-v2-protocol.c \
	src/notification-area-unstable-v2-server-protocol.h \
	$(null)

orbment_plugin_notification_area_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(ORBMENT_CFLAGS) \
	$(WAYLAND_CFLAGS) \
	$(null)

orbment_plugin_notification_area_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	$(MODULES_LDFLAGS) \
	$(null)

orbment_plugin_notification_area_la_LIBADD = \
	$(ORBMENT_LIBS) \
	$(WAYLAND_LIBS) \
	$(null)

orbment-plugin-notification-area.la %D%/src/orbment_plugin_notification_area_la-wlc-notification-area.lo: src/notification-area-unstable-v2-server-protocol.h
