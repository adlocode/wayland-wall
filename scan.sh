#!/bin/sh -e

if [ "x$WAYLAND_SCANNER" = "x" ] ; then
	echo "No scanner present, test skipped." 1>&2
	exit 77
fi

$WAYLAND_SCANNER client-header $1 /dev/null
$WAYLAND_SCANNER server-header $1 /dev/null
$WAYLAND_SCANNER code $1 /dev/null
