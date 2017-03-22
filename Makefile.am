ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

unstable_protocols = \
	unstable/background/background-unstable-v1.xml \
	unstable/background/background-unstable-v2.xml \
	unstable/dock-manager/dock-manager-unstable-v1.xml \
	unstable/dock-manager/dock-manager-unstable-v2.xml \
	unstable/launcher-menu/launcher-menu-unstable-v1.xml \
	unstable/notification-area/notification-area-unstable-v1.xml \
	unstable/window-switcher/window-switcher-unstable-v1.xml \
	$(null)

stable_protocols = \
	$(null)

nobase_dist_pkgdata_DATA = \
	$(unstable_protocols) \
	$(stable_protocols) \
	$(null)

EXTRA_DIST = \
	README.md \
	$(sort $(foreach p,$(unstable_protocols),$(dir $p)README)) \
	$(sort $(foreach p,$(stable_protocols),$(dir $p)README)) \
	$(null)

noarch_pkgconfig_DATA = wayland-wall.pc

dist_check_SCRIPTS = scan.sh

TESTS = $(unstable_protocols) $(stable_protocols)
TEST_EXTENSIONS = .xml
AM_TESTS_ENVIRONMENT = WAYLAND_SCANNER='$(WAYLAND_SCANNER)'; export WAYLAND_SCANNER;
XML_LOG_COMPILER = $(srcdir)/scan.sh

bin_PROGRAMS =

if ENABLE_CLIENTS

bin_PROGRAMS += ww-background

ww_background_SOURCES = \
	src/helpers.h \
	src/background.c \
	src/unstable/background/background-unstable-v2-protocol.c \
	src/stable/viewporter/viewporter-protocol.c \
	$(null)

ww_background_CFLAGS = \
	$(AM_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS) \
	$(WAYLAND_CFLAGS) \
	$(null)

ww_background_LDADD = \
	$(GDK_PIXBUF_LIBS) \
	$(WAYLAND_LIBS) \
	$(null)

ww-background src/ww_background-background.o: src/unstable/background/background-unstable-v2-client-protocol.h src/stable/viewporter/viewporter-client-protocol.h

if ENABLE_TEXT

bin_PROGRAMS += ww-dock

ww_dock_SOURCES = \
	src/helpers.h \
	src/dock.c \
	src/unstable/dock-manager/dock-manager-unstable-v2-protocol.c \
	$(null)

ww_dock_CFLAGS = \
	$(AM_CFLAGS) \
	$(PANGO_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(WAYLAND_CFLAGS) \
	$(null)

ww_dock_LDADD = \
	$(PANGO_LIBS) \
	$(CAIRO_LIBS) \
	$(WAYLAND_LIBS) \
	$(null)

ww-dock src/ww_dock-dock.o: src/unstable/dock-manager/dock-manager-unstable-v2-client-protocol.h

endif

endif

# Wayland protocol code generation rules
src/%-protocol.c: %.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) code < $< > $@

src/%-server-protocol.h: %.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) server-header < $< > $@

src/%-client-protocol.h: %.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) client-header < $< > $@

src/%-protocol.c: $(wpprotocoldir)/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) code < $< > $@

src/%-server-protocol.h: $(wpprotocoldir)/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) server-header < $< > $@

src/%-client-protocol.h: $(wpprotocoldir)/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) client-header < $< > $@
