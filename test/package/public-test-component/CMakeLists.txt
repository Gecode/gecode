cmake_minimum_required(VERSION 3.21)

project(gecode_public_test_component_consumer LANGUAGES CXX)

enable_testing()

find_package(Gecode CONFIG REQUIRED COMPONENTS test)

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

add_test(NAME public-test-component
  COMMAND consumer-smoke -test Package -iter 1 -stop true)
