cmake_minimum_required(VERSION 3.21)
project(OptimizationPackageOriginFixture VERSION 0.1.0 LANGUAGES CXX)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Interface-only fixtures exercise CMake's real generated export behavior.
# Installing them requires no solver compilation or linked binaries.
add_library(gecodeoptimize INTERFACE)
if(PACKAGE_KIND STREQUAL "standalone")
  set(GECODE_OPTIMIZE_WITH_HIGHS OFF)
  configure_package_config_file("${OPTIMIZE_CONFIG_TEMPLATE}"
    "${CMAKE_CURRENT_BINARY_DIR}/GecodeOptimizeConfig.cmake"
    INSTALL_DESTINATION lib/cmake/GecodeOptimize)
  install(TARGETS gecodeoptimize EXPORT GecodeOptimizeTargets)
  install(EXPORT GecodeOptimizeTargets NAMESPACE Gecode:: DESTINATION lib/cmake/GecodeOptimize)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GecodeOptimizeConfig.cmake"
    DESTINATION lib/cmake/GecodeOptimize)
elseif(PACKAGE_KIND STREQUAL "combined")
  add_library(gecodeint INTERFACE)
  add_library(gecodesearch INTERFACE)
  install(TARGETS gecodeint gecodesearch gecodeoptimize EXPORT GecodeTargets)
  install(EXPORT GecodeTargets NAMESPACE Gecode:: DESTINATION lib/cmake/Gecode)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/GecodeConfig.cmake" [=[
include("${CMAKE_CURRENT_LIST_DIR}/GecodeTargets.cmake")
add_library(Gecode::optimize INTERFACE IMPORTED)
set_target_properties(Gecode::optimize PROPERTIES INTERFACE_LINK_LIBRARIES Gecode::gecodeoptimize)
set(Gecode_int_FOUND TRUE)
set(Gecode_search_FOUND TRUE)
]=])
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GecodeConfig.cmake" DESTINATION lib/cmake/Gecode)
else()
  message(FATAL_ERROR "Specify PACKAGE_KIND=standalone or combined")
endif()
