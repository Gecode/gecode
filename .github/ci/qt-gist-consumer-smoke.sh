#!/usr/bin/env bash
set -euxo pipefail

prefix="${1:?usage: qt-gist-consumer-smoke.sh <install-prefix>}"
work="$RUNNER_TEMP/gecode-qt-gist-consumer"
rm -rf "$work"
mkdir -p "$work"

cat > "$work/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.21)
project(qt_gist_consumer LANGUAGES CXX)
find_package(Gecode CONFIG REQUIRED COMPONENTS gist flatzinc)
add_executable(qt_gist_consumer main.cpp)
target_link_libraries(qt_gist_consumer PRIVATE Gecode::gecodegist Gecode::gecodeflatzinc)

function(expect_interface_dependency target expected)
  get_target_property(links "${target}" INTERFACE_LINK_LIBRARIES)
  if(NOT links)
    set(links "")
  endif()
  set(found FALSE)
  foreach(link IN LISTS links)
    if(link STREQUAL "${expected}" OR link STREQUAL "$<LINK_ONLY:${expected}>")
      set(found TRUE)
    endif()
  endforeach()
  if(NOT found)
    message(FATAL_ERROR "Expected ${target} to link ${expected}, got: ${links}")
  endif()
endfunction()

function(expect_gist_variant_dependencies kind qt_widgets_target)
  set(gist_target "Gecode::gecodegist_${kind}")
  if(NOT TARGET "${gist_target}")
    return()
  endif()
  expect_interface_dependency("${gist_target}" "${qt_widgets_target}")
  expect_interface_dependency("${gist_target}" "Gecode::gecodesearch_${kind}")
  expect_interface_dependency("${gist_target}" "Gecode::gecodeint_${kind}")
  if(TARGET "Gecode::gecodeset_${kind}")
    expect_interface_dependency("${gist_target}" "Gecode::gecodeset_${kind}")
  endif()
  if(TARGET "Gecode::gecodefloat_${kind}")
    expect_interface_dependency("${gist_target}" "Gecode::gecodefloat_${kind}")
  endif()
endfunction()

function(expect_variant_dependency component kind expected)
  set(target "Gecode::gecode${component}_${kind}")
  if(TARGET "${target}")
    expect_interface_dependency("${target}" "${expected}")
  endif()
endfunction()

expect_gist_variant_dependencies(shared Qt6::Widgets)
expect_gist_variant_dependencies(static Qt6::Widgets)
expect_variant_dependency(flatzinc shared Qt6::Core)
expect_variant_dependency(flatzinc static Qt6::Core)
EOF
cat > "$work/main.cpp" <<'EOF'
#include <gecode/gist.hh>
#include <gecode/flatzinc.hh>
#include <string>

#ifdef GECODE_HAS_SET_VARS
using SetCompare = std::string (*)(std::string, Gecode::SetVar, Gecode::SetVar);
SetCompare set_compare = &Gecode::Gist::Comparator::compare;
#endif

#ifdef GECODE_HAS_FLOAT_VARS
using FloatCompare = std::string (*)(std::string, Gecode::FloatVar, Gecode::FloatVar);
FloatCompare float_compare = &Gecode::Gist::Comparator::compare;
#endif

int main(void) {
#ifdef GECODE_HAS_SET_VARS
  if (set_compare == nullptr)
    return 1;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
  if (float_compare == nullptr)
    return 1;
#endif
  return 0;
}
EOF

cmake -S "$work" -B "$work/build" -DCMAKE_PREFIX_PATH="$prefix"
cmake --build "$work/build" -j4
