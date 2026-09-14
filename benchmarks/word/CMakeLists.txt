# Optional source tools. These targets are never installed.

foreach(benchmark IN ITEMS
  word-benchmark
  word-bit-network-comparison
  word-dma-descriptor
  word-inverse-arithmetic
  word-register-file)
  add_executable(${benchmark} ${PROJECT_SOURCE_DIR}/examples/${benchmark}.cpp)
  target_link_libraries(${benchmark} PRIVATE gecodeminimodel gecodedriver)
  if(MSVC)
    # The large driver models can exceed the default section limit.
    target_compile_options(${benchmark} PRIVATE /bigobj)
  endif()
endforeach()

foreach(benchmark IN ITEMS
  word-nary-add-benchmark
  word-product-mod-benchmark
  word-symbolic-alu-comparison)
  add_executable(${benchmark} ${benchmark}.cpp)
  target_link_libraries(${benchmark} PRIVATE gecodeword gecodesearch)
endforeach()

add_executable(word-progression-closure
  progression-closure.cpp)
target_link_libraries(word-progression-closure PRIVATE gecodeminimodel)
add_executable(word-distinct-benchmark distinct.cpp)
target_link_libraries(word-distinct-benchmark PRIVATE gecodeminimodel)
add_executable(word-extensional-benchmark extensional.cpp)
target_link_libraries(word-extensional-benchmark PRIVATE gecodeword)
add_executable(word-smt2-reader smt2-reader.cpp)
target_link_libraries(word-smt2-reader PRIVATE gecodeminimodel)
if(BUILD_TESTING)
  find_package(Python3 3.11 QUIET COMPONENTS Interpreter)
  if(Python3_Interpreter_FOUND)
    add_test(NAME build-word-smt2-reader
      COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}"
              --target word-smt2-reader --config $<CONFIG>)
    set_tests_properties(build-word-smt2-reader PROPERTIES
      FIXTURES_SETUP word-smt2-reader-built)
    add_test(NAME word-smt2-reader
      COMMAND ${Python3_EXECUTABLE}
              ${PROJECT_SOURCE_DIR}/benchmarks/word/tests/smt2-reader.py
              $<TARGET_FILE:word-smt2-reader>)
    set_tests_properties(word-smt2-reader PROPERTIES
      FIXTURES_REQUIRED word-smt2-reader-built TIMEOUT 180)
    add_test(NAME word-benchmark-runners
      COMMAND ${Python3_EXECUTABLE}
              ${PROJECT_SOURCE_DIR}/benchmarks/word/test-runners.py)
    set_tests_properties(word-benchmark-runners PROPERTIES TIMEOUT 30)
  endif()
endif()
