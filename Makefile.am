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

lib_LTLIBRARIES =
pkginclude_HEADERS =
pkgconfig_DATA =

dist_pkgdata_DATA += \
	protocol/notification-area-unstable-v1.xml \
	$(null)

noarch_pkgconfig_DATA += \
	pkgconfig/wayland-wall.pc \
	$(null)

CLEANFILES += \
	$(dist_pkgdata_DATA:.xml=-protocol.c) \
	$(dist_pkgdata_DATA:.xml=-server-protocol.h) \
	$(dist_pkgdata_DATA:.xml=-client-protocol.h) \
	$(null)

include weston/weston.mk
include wlc/wlc.mk
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
uninstall-libLTLIBRARIES uninstall-westonpluginLTLIBRARIES uninstall-orbmentpluginLTLIBRARIES uninstall-hook: la-files-uninstall-hook

ww_remove_la = $(if $(strip $($(1)_LTLIBRARIES)),( cd $(DESTDIR)$($(1)dir) && rm -f $($(1)_LTLIBRARIES) ))
ww_remove_so_from_la = $(if $(strip $($(1)_LTLIBRARIES)),for la in $($(1)_LTLIBRARIES); do \
		$(INSTALL) "$(abs_builddir)/$(LT_OBJDIR)"/$${la}i "$(DESTDIR)$($(1)dir)"/$${la}; \
	done )

# *.la files cleanup
la-files-install-hook:
	$(call ww_remove_la,lib)
	$(call ww_remove_la,westonplugin)
	$(call ww_remove_la,orbmentplugin)

# Remove shared object files at uninstall since
# we remove *.la files at install
la-files-uninstall-hook:
	$(call ww_remove_so_from_la,lib)
	$(call ww_remove_so_from_la,westonplugin)
	$(call ww_remove_so_from_la,orbmentplugin)
