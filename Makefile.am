ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

unstable_protocols = \
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
