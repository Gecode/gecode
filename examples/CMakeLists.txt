#
# CMake build script for Gecode examples.
#

set(GECODE_EXAMPLE_SOURCES
  all-interval.cpp
  alpha.cpp
  archimedean-spiral.cpp
  bacp.cpp
  bibd.cpp
  bin-packing.cpp
  black-hole.cpp
  car-sequencing.cpp
  cartesian-heart.cpp
  colored-matrix.cpp
  crew.cpp
  crossword.cpp
  crowded-chess.cpp
  descartes-folium.cpp
  dominating-queens.cpp
  domino.cpp
  donald.cpp
  efpa.cpp
  eq20.cpp
  golden-spiral.cpp
  golf.cpp
  golomb-ruler.cpp
  graph-color.cpp
  grocery.cpp
  hamming.cpp
  ind-set.cpp
  job-shop.cpp
  kakuro.cpp
  knights.cpp
  langford-number.cpp
  magic-sequence.cpp
  magic-square-partial.cpp
  magic-square.cpp
  minesweeper.cpp
  money.cpp
  multi-bin-packing.cpp
  nonogram.cpp
  open-shop.cpp
  ortho-latin.cpp
  partition.cpp
  pentominoes.cpp
  perfect-square.cpp
  photo.cpp
  qcp.cpp
  queen-armies.cpp
  queens.cpp
  radiotherapy.cpp
  sat.cpp
  schurs-lemma.cpp
  sports-league.cpp
  steel-mill.cpp
  steiner.cpp
  sudoku-advanced.cpp
  sudoku.cpp
  tsp.cpp
  warehouses.cpp
  word-square.cpp)

set(GECODE_SET_EXAMPLES
  crew
  golf
  hamming
  queen-armies
  steiner)

set(GECODE_FLOAT_EXAMPLES
  cartesian-heart
  descartes-folium)

set(GECODE_MPFR_FLOAT_EXAMPLES
  archimedean-spiral
  golden-spiral)

function(gecode_add_example source_file)
  get_filename_component(example_name "${source_file}" NAME_WE)

  if(example_name IN_LIST GECODE_SET_EXAMPLES AND NOT GECODE_ENABLE_SET_VARS)
    message(STATUS "Skipping example: ${example_name} (requires set variables)")
    return()
  endif()

  if(example_name IN_LIST GECODE_FLOAT_EXAMPLES AND NOT GECODE_ENABLE_FLOAT_VARS)
    message(STATUS "Skipping example: ${example_name} (requires float variables)")
    return()
  endif()

  if(example_name IN_LIST GECODE_MPFR_FLOAT_EXAMPLES AND
     (NOT GECODE_ENABLE_FLOAT_VARS OR NOT GECODE_ENABLE_MPFR OR NOT MPFR_FOUND))
    message(STATUS "Skipping example: ${example_name} (requires MPFR float support)")
    return()
  endif()

  message(STATUS "Adding example: ${example_name}")
  add_executable(${example_name}${EXECUTABLE_EXTENSION} "${source_file}")
  target_link_libraries(${example_name}${EXECUTABLE_EXTENSION}
    PRIVATE
      gecodeminimodel
      gecodedriver)
  if(MSVC)
    # Needed for some examples to prevent C1128:
    # number of sections exceeded object file format limit.
    target_compile_options(${example_name}${EXECUTABLE_EXTENSION} PRIVATE /bigobj)
  endif()
  set_property(TARGET ${example_name}${EXECUTABLE_EXTENSION} PROPERTY FOLDER "Examples")
endfunction()

foreach(example_source IN LISTS GECODE_EXAMPLE_SOURCES)
  gecode_add_example("${example_source}")
endforeach()
