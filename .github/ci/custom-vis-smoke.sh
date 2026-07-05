#!/usr/bin/env bash
set -euxo pipefail

git clean -ffdx
touch configure

awk '
  /^Name:[[:space:]]*Int$/ { print "Name:           Custom"; next }
  /^Namespace:[[:space:]]*Gecode::Int$/ { print "Namespace:      Gecode::Custom"; next }
  /^Ifdef:[[:space:]]*GECODE_HAS_INT_VARS$/ { next }
  { print }
' gecode/int/var-imp/int.vis > custom.vis

CFLAGS="-fPIC" \
CXXFLAGS="-std=c++11 -fPIC -DGECODE_MEMORY_ALIGNMENT=16" \
  ./configure \
    --with-vis="$PWD/custom.vis" \
    --disable-cpprofiler \
    --disable-examples \
    --disable-flatzinc \
    --disable-float-vars \
    --disable-gist \
    --disable-qt

rm -f gecode/kernel/var-type.hpp gecode/kernel/var-imp.hpp
make -j4
grep -q "CustomVarImpConf" gecode/kernel/var-type.hpp
