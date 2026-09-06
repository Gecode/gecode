cmake_minimum_required(VERSION 3.21)

project(gecode_public_test_component_consumer LANGUAGES CXX)

option(REQUIRE_UNKNOWN_COMPONENT "Request an unsupported Gecode component to verify diagnostics" OFF)

set(CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY ON)
set(CMAKE_FIND_USE_PACKAGE_REGISTRY OFF)

set(_gecode_components test)
if(REQUIRE_UNKNOWN_COMPONENT)
  list(APPEND _gecode_components unsupported_component)
endif()

find_package(Gecode CONFIG REQUIRED COMPONENTS ${_gecode_components})

add_executable(consumer-smoke consumer-smoke.cpp)
target_compile_features(consumer-smoke PRIVATE cxx_std_17)
target_link_libraries(consumer-smoke PRIVATE Gecode::gecodetestint)
if(TARGET Gecode::gecodetestset)
  target_compile_definitions(consumer-smoke PRIVATE GECODE_PACKAGE_HAS_SET)
  target_link_libraries(consumer-smoke PRIVATE Gecode::gecodetestset)
endif()
if(TARGET Gecode::gecodetestfloat)
  target_compile_definitions(consumer-smoke PRIVATE GECODE_PACKAGE_HAS_FLOAT)
  target_link_libraries(consumer-smoke PRIVATE Gecode::gecodetestfloat)
endif()
