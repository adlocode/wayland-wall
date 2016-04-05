AC_PREREQ([2.65])
AC_INIT([wayland-notification-area],[2])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_SRCDIR([protocol/notification-area-unstable-v1.xml])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.aux])

AM_INIT_AUTOMAKE([1.14 foreign subdir-objects parallel-tests dist-xz no-dist-gzip tar-ustar])
AM_SILENT_RULES([yes])

AC_ARG_VAR([WAYLAND_SCANNER], [Wayland XML scanner executable])

AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_MKDIR_P
LT_INIT([disable-static pic-only])
PKG_PROG_PKG_CONFIG
AC_PATH_PROG([WAYLAND_SCANNER], [wayland-scanner])
if test -z "${WAYLAND_SCANNER}"; then
    AC_MSG_ERROR([wayland-scanner not found])
fi
AC_PROG_INSTALL

GETTEXT_PACKAGE=${PACKAGE_NAME}
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["${GETTEXT_PACKAGE}"], [Gettext Package])
AC_SUBST([GETTEXT_PACKAGE])



AC_DEFINE([WNA_EXPORT], [__attribute__((visibility("default")))], [Export symbol])

AM_CFLAGS="-fvisibility=hidden -Wall -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers"
AM_CPPFLAGS=""
AM_LDFLAGS=""


#
# Directories
#

# package-specific dirs
pkgdatadir='${datadir}/'${PACKAGE}
AC_SUBST([pkgdatadir])

# pkg-config files
PKG_NOARCH_INSTALLDIR


#
# Dependencies
#
PKG_CHECK_MODULES([WAYLAND], [wayland-server])

# weston plugin
AC_ARG_ENABLE([weston], AS_HELP_STRING([--disable-weston], [Disable Weston plugin]), [], [enable_weston=yes])
AC_ARG_WITH([westonplugindir], AS_HELP_STRING([--with-westonplugindir=DIR], [Directory for Weston plugins]), [], [with_westonplugindir=yes])
if test x${enable_weston} = xyes; then
    case "${with_westonplugindir}" in
        no|"") AC_MSG_ERROR([*** You must define westonplugindir when compiling Weston plugin]) ;;
        yes) westonplugindir="`${PKG_CONFIG} --variable=libdir weston`/weston" ;;
        *) westonplugindir="${with_westonplugindir}" ;;
    esac
    PKG_CHECK_MODULES([WESTON], [weston pixman-1])
    AC_SUBST([westonplugindir])

    save_CFLAGS=${CFLAGS}
    CFLAGS="${WESTON_CFLAGS} ${WAYLAND_CFLAGS}"
    AC_MSG_CHECKING([if Weston has workarea support])
    AC_COMPILE_IFELSE([
        AC_LANG_PROGRAM(
            [[#include <weston/compositor.h>]],
            [[
                struct weston_compositor *compositor = NULL;
                struct weston_output *output = NULL;
                pixman_rectangle32_t workarea;
                compositor->shell_interface.get_output_work_area(compositor->shell_interface.shell, output, &workarea);
            ]]
        )
    ], [
        AC_MSG_RESULT([yes])
        AC_DEFINE([WESTON_HAS_WORKAREA_SUPPORT], [1], [Weston has workarea support])
    ], [
        AC_MSG_RESULT([no])
    ])
    CFLAGS=${save_CFLAGS}
fi
AM_CONDITIONAL([ENABLE_WESTON], [test x${enable_weston} = xyes])

# orbment plugin
AC_ARG_ENABLE([orbment], AS_HELP_STRING([--disable-orbment], [Disable Orbment plugin]), [], [enable_orbment=yes])
AC_ARG_WITH([orbmentplugindir], AS_HELP_STRING([--with-orbmentplugindir=DIR], [Directory for Orbment plugins]), [], [with_orbmentplugindir=yes])
if test x${enable_orbment} = xyes; then
    case "${with_orbmentplugindir}" in
        no|"") AC_MSG_ERROR([*** You must define orbmentplugindir when compiling Orbment plugin]) ;;
        yes) orbmentplugindir='${libdir}/orbment' ;;
        *) orbmentplugindir="${with_orbmentplugindir}" ;;
    esac
    PKG_CHECK_MODULES([ORBMENT], [wlc])
    AC_CHECK_HEADERS([orbment/plugin.h])
    AS_IF([test x${ac_cv_header_orbment_plugin_h} != xyes], [AC_MSG_ERROR([orbment: plugin.h is required])])
    AC_SUBST([orbmentplugindir])
fi
AM_CONDITIONAL([ENABLE_ORBMENT], [test x${enable_orbment} = xyes])



AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_LDFLAGS])

AM_SUBST_NOTMAKE([LIBS])
AC_SUBST([shrext_cmds])

AC_CONFIG_FILES([
    pkgconfig/wayland-notification-area.pc
    Makefile
])
AC_OUTPUT

AC_MSG_RESULT([
    ${PACKAGE_NAME} ${PACKAGE_VERSION}

    Plugins:
        Weston: ${enable_weston}
        Orbment: ${enable_orbment}
])
