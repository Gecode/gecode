#!/usr/bin/env bash
set -euo pipefail

prefix="${1:?usage: cmake-package-version-checks.sh <install-prefix>}"
temp_root="${RUNNER_TEMP:-${TMPDIR:-/tmp}}"
if [ ! -d "$temp_root" ]; then
  echo "Temporary root is not a directory: $temp_root" >&2
  exit 1
fi

work=
workspace=
cleanup() {
  if [ -n "$work" ]; then
    rm -rf -- "$work"
  fi
}
trap cleanup EXIT

# Record the cleanup boundary before creating anything inside it.  In
# particular, a signal delivered after mktemp creates its directory but before
# command substitution assigns its output can still remove this known parent.
# Ignore termination signals only while ownership of the parent is being
# established.  This prevents cleanup from deleting a colliding path and also
# prevents a signal between mkdir and assignment from leaking a directory we
# just created.
trap '' HUP INT TERM
umask 077
for attempt in {1..100}; do
  candidate="$temp_root/gecode-package-version-checks.${BASHPID:-$$}.$RANDOM.$RANDOM.$attempt"
  if mkdir -m 700 "$candidate"; then
    work=$candidate
    break
  fi
done
unset candidate
trap 'exit 1' HUP INT TERM
if [ -z "$work" ]; then
  echo "Could not create a private package-version workspace below $temp_root" >&2
  exit 1
fi
workspace=$(mktemp -d "$work/run.XXXXXX")
config_version=$(find "$prefix" -type f -name GecodeConfigVersion.cmake -print -quit)

if [ -z "$config_version" ]; then
  echo "GecodeConfigVersion.cmake not found below $prefix" >&2
  exit 1
fi

config_dir=$(dirname "$config_version")
version=$(sed -n 's/^set(PACKAGE_VERSION "\([0-9][0-9.]*\)")$/\1/p' "$config_version" | head -n 1)
IFS=. read -r major minor patch <<<"$version"

if [ -z "${major:-}" ] || [ -z "${minor:-}" ] || [ -z "${patch:-}" ]; then
  echo "Could not read a three-part package version from $config_version" >&2
  exit 1
fi

previous_minor=$((minor - 1))
next_minor=$((minor + 1))
next_patch=$((patch + 1))

mkdir -p "$workspace/source"

cat >"$workspace/source/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(GecodePackageVersionCheck LANGUAGES CXX)

if(REQUEST_MODE STREQUAL "NONE")
  find_package(Gecode CONFIG REQUIRED)
elseif(REQUEST_MODE STREQUAL "EXACT")
  find_package(Gecode ${REQUEST_VERSION} EXACT CONFIG REQUIRED)
else()
  find_package(Gecode ${REQUEST_VERSION} CONFIG REQUIRED)
endif()
EOF

configure_case() {
  name=$1
  package_dir=$2
  mode=$3
  requested=${4:-}
  if [ -n "$requested" ]; then
    cmake -S "$workspace/source" -B "$workspace/$name" \
      -DGecode_DIR="$package_dir" \
      -DREQUEST_MODE="$mode" \
      -DREQUEST_VERSION="$requested"
  else
    cmake -S "$workspace/source" -B "$workspace/$name" \
      -DGecode_DIR="$package_dir" \
      -DREQUEST_MODE="$mode"
  fi
}

check_accepts() {
  if ! configure_case "$@"; then
    echo "Gecode package unexpectedly rejected the $1 case" >&2
    exit 1
  fi
}

check_rejects() {
  name=$1
  requested=$2
  if configure_case "$name" "$config_dir" NORMAL "$requested"; then
    echo "Gecode $version unexpectedly satisfied request $requested" >&2
    exit 1
  fi
}

check_accepts no-version "$config_dir" NONE
check_accepts exact-version "$config_dir" EXACT "$version"
check_rejects previous-minor "$major.$previous_minor.0"
check_rejects next-minor "$major.$next_minor.0"
check_rejects newer-patch "$major.$minor.$next_patch"

# A newer patch release in the same minor line must satisfy this package's
# version, while still rejecting an exact request for the older patch.
mkdir -p "$workspace/newer-patch-package"
cat >"$workspace/newer-patch-package/GecodeConfig.cmake" <<EOF
include("$config_dir/GecodeConfig.cmake")
EOF
sed "s/$version/$major.$minor.$next_patch/g" "$config_version" \
  >"$workspace/newer-patch-package/GecodeConfigVersion.cmake"
check_accepts older-patch-request "$workspace/newer-patch-package" NORMAL "$version"
if configure_case exact-older-patch-request "$workspace/newer-patch-package" EXACT "$version"; then
  echo "A newer patch unexpectedly satisfied an EXACT request for $version" >&2
  exit 1
fi
