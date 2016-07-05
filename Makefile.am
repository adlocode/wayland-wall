ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

unstable_protocols = \
	unstable/background/background-unstable-v1.xml \
	unstable/notification-area/notification-area-unstable-v1.xml \
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
	src/unstable/background/background-unstable-v1-protocol.c \
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

ww-background src/ww_background-background.o: src/unstable/background/background-unstable-v1-client-protocol.h

endif

# Wayland protocol code generation rules
src/%-protocol.c: %.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) code < $< > $@

src/%-server-protocol.h: %.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) server-header < $< > $@

src/%-client-protocol.h: %.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(WAYLAND_SCANNER) client-header < $< > $@
