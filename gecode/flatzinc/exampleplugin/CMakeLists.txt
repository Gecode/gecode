cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(gecode_branch_plugin_example LANGUAGES CXX)
  find_package(Gecode CONFIG REQUIRED COMPONENTS flatzinc)
  set(gecode_plugin_library Gecode::gecodeflatzinc_shared)
else()
  set(gecode_plugin_library gecodeflatzinc_shared)
endif()

if(NOT TARGET ${gecode_plugin_library})
  message(FATAL_ERROR "The branch plugin example requires shared Gecode libraries")
endif()
if(NOT TARGET Qt6::Core AND NOT TARGET Qt5::Core)
  message(FATAL_ERROR "The branch plugin example requires Gecode built with Qt")
endif()

add_library(myplugin MODULE myplugin.cpp myplugin.hh)
set_target_properties(myplugin PROPERTIES
  AUTOMOC ON
  # FlatZinc's loader expects the shared-library suffix, including .dylib on macOS.
  SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(myplugin PRIVATE ${gecode_plugin_library})
