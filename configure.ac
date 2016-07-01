AC_PREREQ([2.65])
AC_INIT([wayland-wall],[1])

AC_CONFIG_SRCDIR([wayland-wall.pc.in])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.aux])

AM_INIT_AUTOMAKE([1.14 foreign subdir-objects parallel-tests dist-xz no-dist-gzip tar-ustar])
AM_SILENT_RULES([yes])

AC_ARG_VAR([WAYLAND_SCANNER], [Wayland XML scanner executable])

AC_PROG_MKDIR_P
PKG_PROG_PKG_CONFIG
AC_PATH_PROG([WAYLAND_SCANNER], [wayland-scanner])
AC_PROG_INSTALL


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
AC_CONFIG_FILES([
    wayland-wall.pc
    Makefile
])
AC_OUTPUT

AC_MSG_RESULT([
    ${PACKAGE_NAME} ${PACKAGE_VERSION}
])
