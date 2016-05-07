if ENABLE_ORBMENT
orbmentplugin_LTLIBRARIES = orbment-plugin-notification-area.la
endif

orbment_plugin_notification_area_la_SOURCES = \
	%D%/src/notification-area.c \
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
	libwlc-notification-area.la \
	$(ORBMENT_LIBS) \
	$(WAYLAND_LIBS) \
	$(null)

orbment-plugin-notification-area.la %D%/src/orbment_plugin_notification_area_la-wlc-notification-area.lo: src/notification-area-unstable-v2-server-protocol.h
