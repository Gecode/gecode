#!/usr/bin/env bash
set -euo pipefail

prefix="${1:?usage: cmake-package-version-checks.sh <install-prefix>}"
config_version=$(find "$prefix" -type f -name GecodeConfigVersion.cmake -print -quit)
if [ -z "$config_version" ]; then
  echo "GecodeConfigVersion.cmake not found below $prefix" >&2
  exit 1
fi

version=$(sed -n 's/^set(PACKAGE_VERSION "\([0-9][0-9.]*\)")$/\1/p' "$config_version" | head -n 1)
IFS=. read -r major minor patch <<<"$version"
if [ -z "${major:-}" ] || [ -z "${minor:-}" ] || [ -z "${patch:-}" ]; then
  echo "Could not read a three-part package version from $config_version" >&2
  exit 1
fi

work=$(mktemp -d "${RUNNER_TEMP:-${TMPDIR:-/tmp}}/gecode-package-version-checks.XXXXXX")
trap 'rm -rf -- "$work"' EXIT
trap 'exit 1' HUP INT TERM

# Exercise the installed compatibility policy without configuring a compiler.
cat > "$work/check.cmake" <<'EOF'
cmake_minimum_required(VERSION 3.21)

# Check how the version file at path answers a request for version: as an
# exact match, as compatible only, or not at all.
function(check_request path request expected)
  string(REPLACE "." ";" parts "${request}")
  list(GET parts 0 PACKAGE_FIND_VERSION_MAJOR)
  list(GET parts 1 PACKAGE_FIND_VERSION_MINOR)
  list(GET parts 2 PACKAGE_FIND_VERSION_PATCH)
  set(PACKAGE_FIND_VERSION "${request}")
  set(PACKAGE_FIND_VERSION_COUNT 3)
  include("${path}")
  if(PACKAGE_VERSION_EXACT)
    set(actual exact)
  elseif(PACKAGE_VERSION_COMPATIBLE)
    set(actual compatible)
  else()
    set(actual rejected)
  endif()
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "Gecode ${PACKAGE_VERSION}: request ${request} was ${actual}, expected ${expected}")
  endif()
endfunction()

math(EXPR previous_minor "${MINOR} - 1")
math(EXPR next_minor "${MINOR} + 1")
math(EXPR next_patch "${PATCH} + 1")
check_request("${CONFIG_VERSION}" "${VERSION}" exact)
check_request("${CONFIG_VERSION}" "${MAJOR}.${previous_minor}.0" rejected)
check_request("${CONFIG_VERSION}" "${MAJOR}.${next_minor}.0" rejected)
check_request("${CONFIG_VERSION}" "${MAJOR}.${MINOR}.${next_patch}" rejected)

# A newer patch release in the same minor line satisfies this version
file(READ "${CONFIG_VERSION}" text)
string(REPLACE "${VERSION}" "${MAJOR}.${MINOR}.${next_patch}" text "${text}")
file(WRITE "${WORK}/newer-patch-version.cmake" "${text}")
check_request("${WORK}/newer-patch-version.cmake" "${VERSION}" compatible)
EOF

cmake "-DCONFIG_VERSION=$config_version" "-DVERSION=$version" "-DWORK=$work" \
  "-DMAJOR=$major" "-DMINOR=$minor" "-DPATCH=$patch" -P "$work/check.cmake"
