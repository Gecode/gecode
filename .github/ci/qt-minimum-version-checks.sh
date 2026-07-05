#!/usr/bin/env bash
set -euxo pipefail

fake_qt="$RUNNER_TEMP/fake-qt5-old"
fake_qt_config="$fake_qt/lib/cmake/Qt5"
mkdir -p "$fake_qt_config"
cat > "$fake_qt_config/Qt5Config.cmake" <<'EOF'
set(QT_VERSION "5.12.0")
set(QT_VERSION_MAJOR 5)
set(Qt5Core_VERSION "5.12.0")
set(Qt5_FOUND TRUE)
set(Qt5Core_FOUND TRUE)
if(NOT TARGET Qt5::Core)
  add_library(Qt5::Core INTERFACE IMPORTED)
endif()
EOF

cmake -S "$GITHUB_WORKSPACE" -B "$RUNNER_TEMP/gecode-old-qt-default" \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_PREFIX_PATH="$fake_qt" \
  -DGECODE_ENABLE_GIST=OFF \
  -DGECODE_ENABLE_FLATZINC=OFF \
  -DGECODE_ENABLE_EXAMPLES=OFF \
  -DGECODE_ENABLE_MPFR=OFF
cmake -S "$GITHUB_WORKSPACE" -B "$RUNNER_TEMP/gecode-old-qt-default" \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_PREFIX_PATH="$fake_qt" \
  -DGECODE_ENABLE_GIST=OFF \
  -DGECODE_ENABLE_FLATZINC=OFF \
  -DGECODE_ENABLE_EXAMPLES=OFF \
  -DGECODE_ENABLE_MPFR=OFF
grep -q '^GECODE_ENABLE_QT:STRING=AUTO' "$RUNNER_TEMP/gecode-old-qt-default/CMakeCache.txt"
grep -Fq '/* #undef GECODE_HAS_QT */' "$RUNNER_TEMP/gecode-old-qt-default/gecode/support/config.hpp"

if cmake -S "$GITHUB_WORKSPACE" -B "$RUNNER_TEMP/gecode-old-qt-explicit" \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_PREFIX_PATH="$fake_qt" \
  -DGECODE_ENABLE_QT=ON \
  -DGECODE_ENABLE_GIST=OFF \
  -DGECODE_ENABLE_FLATZINC=OFF \
  -DGECODE_ENABLE_EXAMPLES=OFF \
  -DGECODE_ENABLE_MPFR=OFF; then
  echo "Expected explicit GECODE_ENABLE_QT=ON with old Qt to fail" >&2
  exit 1
fi
