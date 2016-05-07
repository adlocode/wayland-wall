AM_DISTCHECK_CONFIGURE_FLAGS = \
	--with-westonplugindir="$$dc_install_base/$(westonplugindir)" \
	--with-orbmentplugindir="$$dc_install_base/$(orbmentplugindir)" \
	$(null)

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

dist_pkgdata_DATA =
noarch_pkgconfig_DATA =

EXTRA_DIST = \
	autogen.sh \
	README.md \
	$(null)
CLEANFILES =


AM_CPPFLAGS += \
	-I $(srcdir)/src/ \
	-I $(builddir)/src/ \
	$(null)

AM_CFLAGS += \
	-I $(srcdir)/src/ \
	-I $(builddir)/src/ \
	$(null)

MODULES_LDFLAGS = \
	-avoid-version \
	-module \
	-shared \
	$(null)

dist_pkgdata_DATA += \
	protocol/notification-area-unstable-v2.xml \
	$(null)

noarch_pkgconfig_DATA += \
	pkgconfig/wayland-notification-area.pc \
	$(null)

CLEANFILES += \
	src/notification-area-unstable-v2-protocol.c \
	src/notification-area-unstable-v2-server-protocol.h \
	$(null)

include weston/weston.mk
include orbment/orbment.mk

# Wayland protocol code generation rules
src/%-protocol.c: protocol/%.xml
	$(AM_V_GEN)$(WAYLAND_SCANNER) code < $< > $@

src/%-server-protocol.h: protocol/%.xml
	$(AM_V_GEN)$(WAYLAND_SCANNER) server-header < $< > $@

src/%-client-protocol.h: protocol/%.xml
	$(AM_V_GEN)$(WAYLAND_SCANNER) client-header < $< > $@

-include local-rules.mk

#
# Hooks
#

install-data-hook: la-files-install-hook
uninstall-hook: la-files-uninstall-hook

wna_remove_la = $(if $(strip $($(1)_LTLIBRARIES)),( cd $(DESTDIR)$($(1)dir) && rm -f $($(1)_LTLIBRARIES) ))
wna_remove_so_from_la = $(if $(strip $($(1)_LTLIBRARIES)),( cd $(DESTDIR)$($(1)dir) && rm -f `$(AWK) -F "'" '/^(library_names|old_library)=/ { print $$2 }' $(addprefix "$(abs_builddir)"/,$($(1)_LTLIBRARIES))` ))

# *.la files cleanup
la-files-install-hook:
	$(call wna_remove_la,westonplugin)
	$(call wna_remove_la,orbmentplugin)

# Remove shared object files at uninstall since
# we remove *.la files at install
la-files-uninstall-hook:
	$(call wna_remove_so_from_la,westonplugin)
	$(call wna_remove_so_from_la,orbmentplugin)
