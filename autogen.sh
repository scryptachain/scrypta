#!/bin/sh
set -e
srcdir="$(dirname $0)"
cd "$srcdir"
ls -a
if [ -z ${LIBTOOLIZE} ] && GLIBTOOLIZE="`which glibtoolize 2>/dev/null`"; then
  LIBTOOLIZE="${GLIBTOOLIZE}"
  export LIBTOOLIZE
fi
cd build-aux
autoreconf --install --force --warnings=all
