AC_PREREQ([2.65])
AC_INIT([wayland-wall],[1])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_SRCDIR([wayland-wall.pc.in])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.aux])

AM_INIT_AUTOMAKE([1.14 foreign subdir-objects parallel-tests dist-xz no-dist-gzip tar-ustar])
AM_SILENT_RULES([yes])

AC_ARG_VAR([WAYLAND_SCANNER], [Wayland XML scanner executable])

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_MKDIR_P
PKG_PROG_PKG_CONFIG
AC_PATH_PROG([WAYLAND_SCANNER], [wayland-scanner])
AC_PROG_INSTALL


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

PKG_CHECK_MODULES([WAYLAND_PROTOCLS], [wayland-protocols])
wpprotocoldir=`${PKG_CONFIG} --variable=pkgdatadir wayland-protocols`
AC_SUBST([wpprotocoldir])

wayland_min_version=1.9.91
AC_ARG_ENABLE([clients], AS_HELP_STRING([--enable-clients], [Enable simple clients for some of the protocols]), [], [enable_clients=no])
AC_ARG_ENABLE([images], AS_HELP_STRING([--enable-images], [Enable images support in clients]), [], [enable_images=no])
AC_ARG_ENABLE([text], AS_HELP_STRING([--enable-text], [Enable text support in clients]), [], [enable_text=no])
if test x${enable_clients} = xyes; then
    if test -z "${WAYLAND_SCANNER}"; then
        AC_MSG_ERROR([wayland-scanner not found])
    fi
    AC_FUNC_MMAP
    AC_CHECK_HEADERS([locale.h limits.h stdio.h stdint.h stdbool.h fcntl.h errno.h assert.h sys/mman.h])
    if test x${ac_cv_func_mmap_fixed_mapped} = xno; then
        AC_MSG_ERROR([*** mmap() function is required to build clients])
    fi
    PKG_CHECK_MODULES([WAYLAND], [wayland-client >= ${wayland_min_version} wayland-cursor])
    PKG_CHECK_MODULES([CAIRO], [cairo])
    if test x${enable_images} = xyes; then
        PKG_CHECK_MODULES([GDK_PIXBUF], [gdk-pixbuf-2.0])
        AC_DEFINE([ENABLE_IMAGES], [1], [Images support])
    fi
    if test x${enable_text} = xyes; then
        PKG_CHECK_MODULES([PANGO], [pango pangocairo])
        AC_DEFINE([ENABLE_TEXT], [1], [Text support])
    fi
fi
AM_CONDITIONAL([ENABLE_CLIENTS], [test x${enable_clients} = xyes])
AM_CONDITIONAL([ENABLE_TEXT], [test x${enable_text} = xyes])

AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_LDFLAGS])

AM_SUBST_NOTMAKE([LIBS])
AC_SUBST([shrext_cmds])

AC_CONFIG_FILES([
    wayland-wall.pc
    Makefile
])
AC_OUTPUT

AC_MSG_RESULT([
    ${PACKAGE_NAME} ${PACKAGE_VERSION}

    Simple clients: ${enable_clients}
        Images support: ${enable_images}
        Text support: ${enable_text}
])
