cmake_minimum_required(VERSION 3.21)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(GecodeOptimize VERSION 0.1.0 LANGUAGES C CXX)
  include(CTest)
  include(GNUInstallDirs)
  set(GECODE_OPTIMIZE_STANDALONE ON)
endif()

get_filename_component(GECODE_OPTIMIZE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
option(GECODE_OPTIMIZE_WITH_HIGHS "Enable numerical HiGHS LP/MILP adapter" ON)
set(_optimize_native_default OFF)
if(TARGET gecodeint AND TARGET gecodesearch)
  set(_optimize_native_default ON)
endif()
option(GECODE_OPTIMIZE_WITH_NATIVE "Enable bounded integer Gecode bridge" ${_optimize_native_default})
if(GECODE_OPTIMIZE_WITH_NATIVE AND GECODE_OPTIMIZE_STANDALONE)
  message(FATAL_ERROR "Native optimization bridge requires a top-level Gecode build: configure the repository root with GECODE_ENABLE_OPTIMIZE=ON and enable int and search. Standalone builds support HiGHS or the backend-free core.")
endif()
if(GECODE_OPTIMIZE_WITH_NATIVE AND (NOT TARGET gecodeint OR NOT TARGET gecodesearch))
  message(FATAL_ERROR "Native optimization bridge requires both int and search components")
endif()
option(GECODE_OPTIMIZE_BUILD_TESTS "Build independent optimization conformance tests" ${BUILD_TESTING})
option(GECODE_OPTIMIZE_TEST_PYTHON "Register Python binding tests (disable for sanitizer runtimes without an instrumented Python host)" ON)
set(GECODE_OPTIMIZE_HIGHS_SOURCE "" CACHE PATH "Optional local pinned HiGHS source (no download)")

find_package(Threads REQUIRED)
if(GECODE_OPTIMIZE_WITH_HIGHS)
  if(GECODE_OPTIMIZE_HIGHS_SOURCE)
    # The source is provided explicitly; never fetch at configure/runtime.
    set(_saved_build_testing "${BUILD_TESTING}")
    set(BUILD_TESTING OFF)
    set(BUILD_EXAMPLES OFF)
    set(BUILD_CXX ON)
    set(FAST_BUILD ON)
    add_subdirectory("${GECODE_OPTIMIZE_HIGHS_SOURCE}" "${CMAKE_CURRENT_BINARY_DIR}/highs")
    set(BUILD_TESTING "${_saved_build_testing}")
  else()
    find_package(highs 1.15 CONFIG REQUIRED)
  endif()
endif()

if(DEFINED GECODE_DEFAULT_LINK_VARIANT)
  string(TOUPPER "${GECODE_DEFAULT_LINK_VARIANT}" _optimize_library_type)
elseif(BUILD_SHARED_LIBS)
  set(_optimize_library_type SHARED)
else()
  set(_optimize_library_type STATIC)
endif()
add_library(gecodeoptimize ${_optimize_library_type} model.cpp result.cpp validate.cpp constraints.cpp globals.cpp io.cpp solve.cpp lp_observations.cpp lp_basis.cpp lp_evidence.cpp lp_sensitivity.cpp lp_sensitivity_backend.cpp native.cpp native_presolve.cpp native_components.cpp native_symmetry.cpp diagnostics.cpp workflow.cpp scenarios.cpp relaxation.cpp pool.cpp presolve.cpp flatzinc.cpp quadratic.cpp quadratic_bound.cpp quadratic_solve.cpp)
add_library(Gecode::gecodeoptimize ALIAS gecodeoptimize)
add_library(Gecode::optimize ALIAS gecodeoptimize)
target_compile_features(gecodeoptimize PUBLIC cxx_std_17)
target_include_directories(gecodeoptimize PUBLIC
  $<BUILD_INTERFACE:${GECODE_OPTIMIZE_ROOT}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(gecodeoptimize PUBLIC Threads::Threads)
if(GECODE_OPTIMIZE_WITH_HIGHS)
  target_compile_definitions(gecodeoptimize PRIVATE GECODE_OPTIMIZE_WITH_HIGHS=1)
  target_link_libraries(gecodeoptimize PRIVATE highs::highs)
endif()
if(GECODE_OPTIMIZE_WITH_NATIVE)
  target_compile_definitions(gecodeoptimize PRIVATE GECODE_OPTIMIZE_WITH_NATIVE=1)
  target_link_libraries(gecodeoptimize PRIVATE gecodeint gecodesearch)
  # ELF shared C wrappers cannot embed non-PIC native archives. Keep the
  # normal native-only build policy unchanged unless this component needs them.
  if(UNIX AND NOT APPLE)
    foreach(component support kernel search int)
      if(TARGET gecode${component}_static)
        set_property(TARGET gecode${component}_static PROPERTY POSITION_INDEPENDENT_CODE ON)
      endif()
    endforeach()
  endif()
endif()
set_target_properties(gecodeoptimize PROPERTIES POSITION_INDEPENDENT_CODE ON WINDOWS_EXPORT_ALL_SYMBOLS ON)
# The C/Python boundary is always a shared library, including a static C++ build.
# Its explicit C exports are independent of internal C++ symbol visibility.
add_library(gecodeoptimize_c SHARED c_api.cpp)
add_library(Gecode::gecodeoptimize_c ALIAS gecodeoptimize_c)
add_library(Gecode::optimize_c ALIAS gecodeoptimize_c)
target_compile_features(gecodeoptimize_c PRIVATE cxx_std_17)
target_compile_definitions(gecodeoptimize_c PRIVATE GECODE_OPT_C_API_EXPORTS)
target_include_directories(gecodeoptimize_c PUBLIC
  $<BUILD_INTERFACE:${GECODE_OPTIMIZE_ROOT}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(gecodeoptimize_c PRIVATE gecodeoptimize)
# Keep facade RTTI visible across shared-library boundaries: hiding ModelError's
# typeinfo in this wrapper breaks its exception classification on Apple libc++.
set_target_properties(gecodeoptimize_c PROPERTIES VERSION 1.0.0 SOVERSION 1)
if(APPLE)
  set_target_properties(gecodeoptimize gecodeoptimize_c PROPERTIES INSTALL_RPATH "@loader_path")
elseif(UNIX)
  set_target_properties(gecodeoptimize gecodeoptimize_c PROPERTIES INSTALL_RPATH "$ORIGIN")
endif()

if(GECODE_OPTIMIZE_BUILD_TESTS)
  enable_testing()
  # Independent oracles retain their original exhaustive fixtures and do not
  # depend on benchmark drivers, measurements or generated data.
  function(gecode_optimize_add_oracle name source)
    add_executable(optimize-${name}-test "${GECODE_OPTIMIZE_ROOT}/test/optimize/${source}")
    target_compile_features(optimize-${name}-test PRIVATE cxx_std_17)
    target_compile_definitions(optimize-${name}-test PRIVATE GECODE_NO_AUTOLINK)
    target_include_directories(optimize-${name}-test PRIVATE "${CMAKE_BINARY_DIR}" "${GECODE_OPTIMIZE_ROOT}")
    if(MSVC)
      target_compile_options(optimize-${name}-test PRIVATE /UNDEBUG)
    else()
      target_compile_options(optimize-${name}-test PRIVATE -UNDEBUG)
    endif()
    add_test(NAME optimize-${name} COMMAND optimize-${name}-test)
    set_tests_properties(optimize-${name} PROPERTIES TIMEOUT 60)
  endfunction()
  foreach(oracle certificate integer_certificate sparse_certificate)
    string(REPLACE "_" "-" oracle_name "${oracle}")
    gecode_optimize_add_oracle(lp-${oracle_name} lp_${oracle}.cpp)
  endforeach()
  foreach(test_name model bulk flatzinc quadratic quadratic_bound result validate constraints globals regular io solve lp_observations lp_observations_checks lp_basis lp_evidence lp_sensitivity session session_limits native native_auto native_race native_presolve native_components native_symmetry native_lp native_search native_knapsack native_branching native_starts native_neighborhoods diagnostics workflow scenarios relaxation pool presolve presolve_solve)
    add_executable(optimize-${test_name}-test "${GECODE_OPTIMIZE_ROOT}/test/optimize/${test_name}.cpp")
    target_link_libraries(optimize-${test_name}-test PRIVATE gecodeoptimize)
    if(MSVC)
      target_compile_options(optimize-${test_name}-test PRIVATE /UNDEBUG)
    else()
      target_compile_options(optimize-${test_name}-test PRIVATE -UNDEBUG)
    endif()
    add_test(NAME optimize-${test_name} COMMAND optimize-${test_name}-test)
    set_tests_properties(optimize-${test_name} PROPERTIES TIMEOUT 60)
  endforeach()
  # Full-source Debug sanitizers make this nontrivial MIP lifetime fixture
  # substantially slower. It is separate from the bounded fast regression gate.
  set_tests_properties(optimize-session_limits PROPERTIES TIMEOUT 180)
  if(GECODE_OPTIMIZE_WITH_HIGHS)
    # Dirty the real backend before the separately compiled rejection seam.
    # The production adapter and all ordinary solves contain no test selector.
    add_library(optimize-lp-basis-failure-adapter OBJECT solve.cpp)
    target_compile_features(optimize-lp-basis-failure-adapter PRIVATE cxx_std_17)
    target_include_directories(optimize-lp-basis-failure-adapter PRIVATE "${GECODE_OPTIMIZE_ROOT}")
    target_compile_definitions(optimize-lp-basis-failure-adapter PRIVATE
      GECODE_OPTIMIZE_WITH_HIGHS=1 GECODE_OPTIMIZE_TEST_LP_BASIS_FAILURE=1)
    target_link_libraries(optimize-lp-basis-failure-adapter PRIVATE highs::highs Threads::Threads)
    add_executable(optimize-lp-basis-failure-test
      "${GECODE_OPTIMIZE_ROOT}/test/optimize/lp_basis_failure.cpp"
      model.cpp result.cpp validate.cpp constraints.cpp globals.cpp native.cpp native_presolve.cpp native_components.cpp native_symmetry.cpp presolve.cpp
      lp_observations.cpp lp_basis.cpp $<TARGET_OBJECTS:optimize-lp-basis-failure-adapter>)
    target_compile_features(optimize-lp-basis-failure-test PRIVATE cxx_std_17)
    target_include_directories(optimize-lp-basis-failure-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
    target_link_libraries(optimize-lp-basis-failure-test PRIVATE highs::highs Threads::Threads)
    if(MSVC)
      target_compile_options(optimize-lp-basis-failure-test PRIVATE /UNDEBUG)
    else()
      target_compile_options(optimize-lp-basis-failure-test PRIVATE -UNDEBUG)
    endif()
    add_test(NAME optimize-lp-basis-failure COMMAND optimize-lp-basis-failure-test)
    set_tests_properties(optimize-lp-basis-failure PROPERTIES TIMEOUT 60)
  endif()
  # This executable owns its substitute backend and lifecycle hooks; neither
  # they nor the experimental regularization override enter the public library.
  add_executable(optimize-quadratic-coordinator-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/quadratic.cpp"
    model.cpp result.cpp validate.cpp constraints.cpp globals.cpp
    quadratic.cpp quadratic_bound.cpp quadratic_solve.cpp)
  target_compile_features(optimize-quadratic-coordinator-test PRIVATE cxx_std_17)
  target_include_directories(optimize-quadratic-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
  target_compile_definitions(optimize-quadratic-coordinator-test PRIVATE GECODE_QUADRATIC_TEST_HOOKS=1)
  target_link_libraries(optimize-quadratic-coordinator-test PRIVATE Threads::Threads)
  if(GECODE_OPTIMIZE_WITH_HIGHS)
    target_compile_definitions(optimize-quadratic-coordinator-test PRIVATE GECODE_OPTIMIZE_WITH_HIGHS=1)
    target_link_libraries(optimize-quadratic-coordinator-test PRIVATE highs::highs)
  endif()
  if(MSVC)
    target_compile_options(optimize-quadratic-coordinator-test PRIVATE /UNDEBUG)
  else()
    target_compile_options(optimize-quadratic-coordinator-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME optimize-quadratic-coordinator COMMAND optimize-quadratic-coordinator-test)
  set_tests_properties(optimize-quadratic optimize-quadratic-coordinator PROPERTIES TIMEOUT 180)
  if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$" AND NOT MSVC)
    # Compile only the checker with unsafe arithmetic. Keep the harness and
    # foundation normal so this verifies rejection, not a corrupted test oracle.
    add_library(optimize-quadratic-fast-math OBJECT quadratic_bound.cpp)
    target_compile_features(optimize-quadratic-fast-math PRIVATE cxx_std_17)
    target_include_directories(optimize-quadratic-fast-math PRIVATE "${GECODE_OPTIMIZE_ROOT}")
    target_compile_options(optimize-quadratic-fast-math PRIVATE -ffast-math)
    add_executable(optimize-quadratic-arithmetic-rejection-test
      "${GECODE_OPTIMIZE_ROOT}/test/optimize/quadratic_bound.cpp"
      model.cpp result.cpp validate.cpp constraints.cpp globals.cpp quadratic.cpp
      $<TARGET_OBJECTS:optimize-quadratic-fast-math>)
    target_compile_features(optimize-quadratic-arithmetic-rejection-test PRIVATE cxx_std_17)
    target_include_directories(optimize-quadratic-arithmetic-rejection-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
    target_compile_definitions(optimize-quadratic-arithmetic-rejection-test PRIVATE
      GECODE_QUADRATIC_EXPECT_UNSUPPORTED_ARITHMETIC=1)
    target_compile_options(optimize-quadratic-arithmetic-rejection-test PRIVATE -UNDEBUG)
    target_link_libraries(optimize-quadratic-arithmetic-rejection-test PRIVATE Threads::Threads)
    add_test(NAME optimize-quadratic-arithmetic-rejection COMMAND optimize-quadratic-arithmetic-rejection-test)
    set_tests_properties(optimize-quadratic-arithmetic-rejection PROPERTIES TIMEOUT 60)
  endif()
  if(GECODE_OPTIMIZE_WITH_NATIVE)
    target_compile_definitions(optimize-native-test PRIVATE GECODE_OPTIMIZE_WITH_NATIVE=1)
    add_executable(optimize-brancher-lifecycle-test "${GECODE_OPTIMIZE_ROOT}/test/optimize/brancher_lifecycle.cpp")
    target_link_libraries(optimize-brancher-lifecycle-test PRIVATE gecodeint gecodesearch)
    add_test(NAME optimize-brancher-lifecycle COMMAND optimize-brancher-lifecycle-test)
    set_tests_properties(optimize-brancher-lifecycle PROPERTIES TIMEOUT 60)
    gecode_optimize_add_oracle(search-checkpoint search_checkpoint.cpp)
    target_link_libraries(optimize-search-checkpoint-test PRIVATE gecodeint gecodesearch)
    # These proof checkers use native configuration headers but no solver.
    gecode_optimize_add_oracle(cut-proofs cuts.cpp)
    gecode_optimize_add_oracle(lp-strengthening lp_strengthening.cpp)
    if(GECODE_OPTIMIZE_WITH_HIGHS)
      include(CheckCXXSourceCompiles)
      check_cxx_source_compiles("#if !defined(__SIZEOF_INT128__) || !(defined(__GNUC__) || defined(__clang__))
#error Checked wide integer certificates are unavailable
#endif
int main() { return 0; }" GECODE_OPTIMIZE_HAVE_CHECKED_WIDE)
      if(GECODE_OPTIMIZE_HAVE_CHECKED_WIDE)
        # Header-only coordinator: hooks exist only in this oracle executable.
        gecode_optimize_add_oracle(cut-loop cut_loop.cpp)
        target_compile_definitions(optimize-cut-loop-test PRIVATE
          GECODE_LP_CUT_LOOP_TEST_NATIVE=1 GECODE_LP_CUT_LOOP_TEST_HOOKS=1)
        target_link_libraries(optimize-cut-loop-test PRIVATE gecodeint gecodesearch highs::highs Threads::Threads)
        set_tests_properties(optimize-cut-loop PROPERTIES TIMEOUT 120)
        foreach(oracle backend integer_backend propagator integer_propagator sparse_storage reduced_cost)
          string(REPLACE "_" "-" oracle_name "${oracle}")
          gecode_optimize_add_oracle(lp-${oracle_name} lp_${oracle}.cpp)
          target_link_libraries(optimize-lp-${oracle_name}-test PRIVATE gecodeint gecodesearch highs::highs Threads::Threads)
        endforeach()
      endif()
    endif()
    add_executable(optimize-native-search-coordinator-test
      "${GECODE_OPTIMIZE_ROOT}/test/optimize/native_search.cpp"
      model.cpp result.cpp validate.cpp constraints.cpp globals.cpp native.cpp native_presolve.cpp native_components.cpp native_symmetry.cpp presolve.cpp)
    target_compile_features(optimize-native-search-coordinator-test PRIVATE cxx_std_17)
    target_include_directories(optimize-native-search-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
    target_compile_definitions(optimize-native-search-coordinator-test PRIVATE
      GECODE_NATIVE_SEARCH_TEST_HOOKS=1 GECODE_NATIVE_ROOT_CUT_TEST_HOOKS=1 GECODE_OPTIMIZE_WITH_NATIVE=1)
    target_link_libraries(optimize-native-search-coordinator-test PRIVATE gecodeint gecodesearch Threads::Threads)
    if(GECODE_OPTIMIZE_WITH_HIGHS)
      target_compile_definitions(optimize-native-search-coordinator-test PRIVATE GECODE_OPTIMIZE_WITH_HIGHS=1)
      target_link_libraries(optimize-native-search-coordinator-test PRIVATE highs::highs)
    endif()
    add_test(NAME optimize-native-search-coordinator COMMAND optimize-native-search-coordinator-test)
    set_tests_properties(optimize-native-search-coordinator PROPERTIES TIMEOUT 120)
    add_executable(optimize-native-lp-coordinator-test
      "${GECODE_OPTIMIZE_ROOT}/test/optimize/native_lp.cpp"
      model.cpp result.cpp validate.cpp constraints.cpp globals.cpp native.cpp native_presolve.cpp native_components.cpp native_symmetry.cpp presolve.cpp)
    target_compile_features(optimize-native-lp-coordinator-test PRIVATE cxx_std_17)
    target_include_directories(optimize-native-lp-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
    target_compile_definitions(optimize-native-lp-coordinator-test PRIVATE
      GECODE_NATIVE_ROOT_CUT_TEST_HOOKS=1 GECODE_OPTIMIZE_WITH_NATIVE=1)
    target_link_libraries(optimize-native-lp-coordinator-test PRIVATE gecodeint gecodesearch Threads::Threads)
    if(GECODE_OPTIMIZE_WITH_HIGHS)
      target_compile_definitions(optimize-native-lp-coordinator-test PRIVATE GECODE_OPTIMIZE_WITH_HIGHS=1)
      target_link_libraries(optimize-native-lp-coordinator-test PRIVATE highs::highs)
    endif()
    add_test(NAME optimize-native-lp-coordinator COMMAND optimize-native-lp-coordinator-test)
    set_tests_properties(optimize-native-lp-coordinator PROPERTIES TIMEOUT 120)
    add_executable(optimize-native-branching-coordinator-test
      "${GECODE_OPTIMIZE_ROOT}/test/optimize/native_branching.cpp"
      model.cpp result.cpp validate.cpp constraints.cpp globals.cpp native.cpp native_presolve.cpp native_components.cpp native_symmetry.cpp presolve.cpp)
    target_compile_features(optimize-native-branching-coordinator-test PRIVATE cxx_std_17)
    target_include_directories(optimize-native-branching-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
    target_compile_definitions(optimize-native-branching-coordinator-test PRIVATE
      GECODE_NATIVE_BRANCHING_TEST_HOOKS=1 GECODE_NATIVE_SEARCH_TEST_HOOKS=1 GECODE_OPTIMIZE_WITH_NATIVE=1)
    target_link_libraries(optimize-native-branching-coordinator-test PRIVATE gecodeint gecodesearch Threads::Threads)
    if(GECODE_OPTIMIZE_WITH_HIGHS)
      target_compile_definitions(optimize-native-branching-coordinator-test PRIVATE GECODE_OPTIMIZE_WITH_HIGHS=1)
      target_link_libraries(optimize-native-branching-coordinator-test PRIVATE highs::highs)
    endif()
    add_test(NAME optimize-native-branching-coordinator COMMAND optimize-native-branching-coordinator-test)
    set_tests_properties(optimize-native_branching optimize-native-branching-coordinator PROPERTIES TIMEOUT 120)
    add_executable(optimize-native-neighborhood-coordinator-test
      "${GECODE_OPTIMIZE_ROOT}/test/optimize/native_neighborhoods.cpp"
      model.cpp result.cpp validate.cpp constraints.cpp globals.cpp native.cpp native_presolve.cpp native_components.cpp native_symmetry.cpp presolve.cpp)
    target_compile_features(optimize-native-neighborhood-coordinator-test PRIVATE cxx_std_17)
    target_include_directories(optimize-native-neighborhood-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
    target_compile_definitions(optimize-native-neighborhood-coordinator-test PRIVATE
      GECODE_NATIVE_NEIGHBORHOOD_TEST_HOOKS=1 GECODE_OPTIMIZE_WITH_NATIVE=1)
    target_link_libraries(optimize-native-neighborhood-coordinator-test PRIVATE gecodeint gecodesearch Threads::Threads)
    if(GECODE_OPTIMIZE_WITH_HIGHS)
      target_compile_definitions(optimize-native-neighborhood-coordinator-test PRIVATE GECODE_OPTIMIZE_WITH_HIGHS=1)
      target_link_libraries(optimize-native-neighborhood-coordinator-test PRIVATE highs::highs)
    endif()
    add_test(NAME optimize-native-neighborhood-coordinator COMMAND optimize-native-neighborhood-coordinator-test)
    set_tests_properties(optimize-native_neighborhoods optimize-native-neighborhood-coordinator PROPERTIES TIMEOUT 120)
    add_executable(optimize-native-start-coordinator-test
      "${GECODE_OPTIMIZE_ROOT}/test/optimize/native_starts.cpp"
      model.cpp result.cpp validate.cpp constraints.cpp globals.cpp native.cpp native_presolve.cpp native_components.cpp native_symmetry.cpp presolve.cpp)
    target_compile_features(optimize-native-start-coordinator-test PRIVATE cxx_std_17)
    target_include_directories(optimize-native-start-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
    target_compile_definitions(optimize-native-start-coordinator-test PRIVATE
      GECODE_NATIVE_START_TEST_HOOKS=1 GECODE_NATIVE_SEARCH_TEST_HOOKS=1 GECODE_NATIVE_ROOT_CUT_TEST_HOOKS=1 GECODE_OPTIMIZE_WITH_NATIVE=1)
    target_link_libraries(optimize-native-start-coordinator-test PRIVATE gecodeint gecodesearch Threads::Threads)
    if(GECODE_OPTIMIZE_WITH_HIGHS)
      target_compile_definitions(optimize-native-start-coordinator-test PRIVATE GECODE_OPTIMIZE_WITH_HIGHS=1)
      target_link_libraries(optimize-native-start-coordinator-test PRIVATE highs::highs)
    endif()
    add_test(NAME optimize-native-start-coordinator COMMAND optimize-native-start-coordinator-test)
    set_tests_properties(optimize-native_starts optimize-native-start-coordinator PROPERTIES TIMEOUT 120)
  endif()
  if(GECODE_OPTIMIZE_WITH_HIGHS)
    target_compile_definitions(optimize-io-test PRIVATE GECODE_OPTIMIZE_IO_TEST_HIGHS=1)
    target_link_libraries(optimize-io-test PRIVATE highs::highs)
  endif()
  set_tests_properties(optimize-io PROPERTIES
    WORKING_DIRECTORY "${GECODE_OPTIMIZE_ROOT}")
  # A deterministic substitute backend tests interruptions and invalid stage
  # reports without timing-dependent sleeps or reliance on solver heuristics.
  add_executable(optimize-workflow-coordinator-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/workflow.cpp"
    model.cpp result.cpp validate.cpp constraints.cpp globals.cpp workflow.cpp)
  target_compile_features(optimize-workflow-coordinator-test PRIVATE cxx_std_17)
  target_include_directories(optimize-workflow-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
  target_compile_definitions(optimize-workflow-coordinator-test PRIVATE GECODE_WORKFLOW_TEST_FAKE_SOLVER=1)
  target_link_libraries(optimize-workflow-coordinator-test PRIVATE Threads::Threads)
  if(MSVC)
    target_compile_options(optimize-workflow-coordinator-test PRIVATE /UNDEBUG)
  else()
    target_compile_options(optimize-workflow-coordinator-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME optimize-workflow-coordinator COMMAND optimize-workflow-coordinator-test)
  set_tests_properties(optimize-workflow-coordinator PROPERTIES TIMEOUT 60)
  add_executable(optimize-diagnostics-coordinator-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/diagnostics.cpp"
    model.cpp result.cpp validate.cpp constraints.cpp globals.cpp diagnostics.cpp)
  target_compile_features(optimize-diagnostics-coordinator-test PRIVATE cxx_std_17)
  target_include_directories(optimize-diagnostics-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
  target_compile_definitions(optimize-diagnostics-coordinator-test PRIVATE GECODE_DIAGNOSTICS_TEST_FAKE_SOLVER=1)
  target_link_libraries(optimize-diagnostics-coordinator-test PRIVATE Threads::Threads)
  if(MSVC)
    target_compile_options(optimize-diagnostics-coordinator-test PRIVATE /UNDEBUG)
  else()
    target_compile_options(optimize-diagnostics-coordinator-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME optimize-diagnostics-coordinator COMMAND optimize-diagnostics-coordinator-test)
  set_tests_properties(optimize-diagnostics-coordinator PROPERTIES TIMEOUT 60)
  add_executable(optimize-relaxation-coordinator-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/relaxation.cpp"
    model.cpp result.cpp validate.cpp constraints.cpp globals.cpp workflow.cpp relaxation.cpp)
  target_compile_features(optimize-relaxation-coordinator-test PRIVATE cxx_std_17)
  target_include_directories(optimize-relaxation-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
  target_compile_definitions(optimize-relaxation-coordinator-test PRIVATE GECODE_RELAXATION_TEST_FAKE_SOLVER=1)
  target_link_libraries(optimize-relaxation-coordinator-test PRIVATE Threads::Threads)
  if(MSVC)
    target_compile_options(optimize-relaxation-coordinator-test PRIVATE /UNDEBUG)
  else()
    target_compile_options(optimize-relaxation-coordinator-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME optimize-relaxation-coordinator COMMAND optimize-relaxation-coordinator-test)
  set_tests_properties(optimize-relaxation-coordinator PROPERTIES TIMEOUT 60)
  add_executable(optimize-pool-coordinator-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/pool.cpp"
    model.cpp result.cpp validate.cpp constraints.cpp globals.cpp pool.cpp)
  target_compile_features(optimize-pool-coordinator-test PRIVATE cxx_std_17)
  target_include_directories(optimize-pool-coordinator-test PRIVATE "${GECODE_OPTIMIZE_ROOT}")
  target_compile_definitions(optimize-pool-coordinator-test PRIVATE GECODE_POOL_TEST_FAKE_SOLVER=1)
  target_link_libraries(optimize-pool-coordinator-test PRIVATE Threads::Threads)
  if(MSVC)
    target_compile_options(optimize-pool-coordinator-test PRIVATE /UNDEBUG)
  else()
    target_compile_options(optimize-pool-coordinator-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME optimize-pool-coordinator COMMAND optimize-pool-coordinator-test)
  set_tests_properties(optimize-pool-coordinator PROPERTIES TIMEOUT 60)
  # Only this executable has substitute scenario solves and checkpoint hooks.
  # Link the ordinary library for shared model/session support; its separate
  # scenarios object is not pulled into a static link by these definitions.
  add_executable(optimize-scenarios-coordinator-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/scenarios_coordinator.cpp" scenarios.cpp)
  target_compile_definitions(optimize-scenarios-coordinator-test PRIVATE GECODE_OPTIMIZE_TEST_SCENARIOS=1)
  target_link_libraries(optimize-scenarios-coordinator-test PRIVATE gecodeoptimize)
  if(MSVC)
    target_compile_options(optimize-scenarios-coordinator-test PRIVATE /UNDEBUG)
  else()
    target_compile_options(optimize-scenarios-coordinator-test PRIVATE -UNDEBUG)
  endif()
  add_test(NAME optimize-scenarios-coordinator COMMAND optimize-scenarios-coordinator-test)
  set_tests_properties(optimize-scenarios-coordinator PROPERTIES TIMEOUT 60)
  add_executable(optimize-lp-evidence-coordinator-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/lp_evidence_coordinator.cpp" lp_evidence.cpp)
  target_compile_definitions(optimize-lp-evidence-coordinator-test PRIVATE GECODE_OPTIMIZE_TEST_LP_EVIDENCE=1)
  target_link_libraries(optimize-lp-evidence-coordinator-test PRIVATE gecodeoptimize)
  add_test(NAME optimize-lp-evidence-coordinator COMMAND optimize-lp-evidence-coordinator-test)
  set_tests_properties(optimize-lp-evidence-coordinator PROPERTIES TIMEOUT 60)
  # The substitute factor and lifecycle checkpoints exist only in this test's
  # separately compiled analyzer, never in the product library.
  add_executable(optimize-lp-sensitivity-coordinator-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/lp_sensitivity_coordinator.cpp" lp_sensitivity.cpp)
  target_compile_definitions(optimize-lp-sensitivity-coordinator-test PRIVATE GECODE_OPTIMIZE_TEST_LP_SENSITIVITY=1)
  target_link_libraries(optimize-lp-sensitivity-coordinator-test PRIVATE gecodeoptimize)
  add_test(NAME optimize-lp-sensitivity-coordinator COMMAND optimize-lp-sensitivity-coordinator-test)
  set_tests_properties(optimize-lp-sensitivity-coordinator PROPERTIES TIMEOUT 60)
  if(GECODE_OPTIMIZE_WITH_HIGHS)
    add_executable(optimize-lp-sensitivity-factor-test
      "${GECODE_OPTIMIZE_ROOT}/test/optimize/lp_sensitivity_factor.cpp")
    target_compile_features(optimize-lp-sensitivity-factor-test PRIVATE cxx_std_17)
    target_link_libraries(optimize-lp-sensitivity-factor-test PRIVATE highs::highs)
    add_test(NAME optimize-lp-sensitivity-factor COMMAND optimize-lp-sensitivity-factor-test)
    set_tests_properties(optimize-lp-sensitivity-factor PROPERTIES TIMEOUT 60)
  endif()
  add_executable(optimize-lp-sensitivity-binding-cleanup-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/lp_sensitivity_binding_cleanup.cpp" c_api.cpp)
  target_compile_definitions(optimize-lp-sensitivity-binding-cleanup-test PRIVATE
    GECODE_OPTIMIZE_TEST_SENSITIVITY_BINDING=1 GECODE_OPT_C_API_EXPORTS)
  target_link_libraries(optimize-lp-sensitivity-binding-cleanup-test PRIVATE gecodeoptimize)
  add_test(NAME optimize-lp-sensitivity-binding-cleanup COMMAND optimize-lp-sensitivity-binding-cleanup-test)
  set_tests_properties(optimize-lp-sensitivity-binding-cleanup PROPERTIES TIMEOUT 60)
  add_executable(optimize-lp-sensitivity-c-test "${GECODE_OPTIMIZE_ROOT}/test/optimize/lp_sensitivity_c.c")
  set_target_properties(optimize-lp-sensitivity-c-test PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
  target_link_libraries(optimize-lp-sensitivity-c-test PRIVATE gecodeoptimize_c)
  add_test(NAME optimize-lp-sensitivity-c COMMAND optimize-lp-sensitivity-c-test)
  set_tests_properties(optimize-lp-sensitivity-c PROPERTIES TIMEOUT 60)
  add_executable(optimize-lp-evidence-binding-cleanup-test
    "${GECODE_OPTIMIZE_ROOT}/test/optimize/lp_evidence_binding_cleanup.cpp" c_api.cpp)
  target_compile_definitions(optimize-lp-evidence-binding-cleanup-test PRIVATE
    GECODE_OPTIMIZE_TEST_EVIDENCE_BINDING=1 GECODE_OPT_C_API_EXPORTS)
  target_link_libraries(optimize-lp-evidence-binding-cleanup-test PRIVATE gecodeoptimize)
  add_test(NAME optimize-lp-evidence-binding-cleanup COMMAND optimize-lp-evidence-binding-cleanup-test)
  set_tests_properties(optimize-lp-evidence-binding-cleanup PROPERTIES TIMEOUT 60)
  add_executable(optimize-lp-evidence-c-test "${GECODE_OPTIMIZE_ROOT}/test/optimize/lp_evidence_c.c")
  set_target_properties(optimize-lp-evidence-c-test PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
  target_link_libraries(optimize-lp-evidence-c-test PRIVATE gecodeoptimize_c)
  add_test(NAME optimize-lp-evidence-c COMMAND optimize-lp-evidence-c-test)
  set_tests_properties(optimize-lp-evidence-c PROPERTIES TIMEOUT 60)
  add_executable(optimize-c-api-test "${GECODE_OPTIMIZE_ROOT}/test/optimize/c_api.c")
  set_target_properties(optimize-c-api-test PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
  target_link_libraries(optimize-c-api-test PRIVATE gecodeoptimize_c)
  add_test(NAME optimize-c-api COMMAND optimize-c-api-test)
  set_tests_properties(optimize-c-api PROPERTIES TIMEOUT 60)
  add_executable(optimize-lp-observations-c-test "${GECODE_OPTIMIZE_ROOT}/test/optimize/lp_observations_c.c")
  set_target_properties(optimize-lp-observations-c-test PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
  target_link_libraries(optimize-lp-observations-c-test PRIVATE gecodeoptimize_c)
  add_test(NAME optimize-lp-observations-c COMMAND optimize-lp-observations-c-test)
  set_tests_properties(optimize-lp-observations-c PROPERTIES TIMEOUT 60)
  add_executable(optimize-scenarios-c-test "${GECODE_OPTIMIZE_ROOT}/test/optimize/scenarios_c.c")
  set_target_properties(optimize-scenarios-c-test PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
  target_link_libraries(optimize-scenarios-c-test PRIVATE gecodeoptimize_c)
  add_test(NAME optimize-scenarios-c COMMAND optimize-scenarios-c-test)
  set_tests_properties(optimize-scenarios-c PROPERTIES TIMEOUT 60)
  foreach(c_test lp_basis regular)
    string(REPLACE "_" "-" c_test_name "${c_test}")
    add_executable(optimize-${c_test_name}-c-api-test "${GECODE_OPTIMIZE_ROOT}/test/optimize/${c_test}_c_api.c")
    set_target_properties(optimize-${c_test_name}-c-api-test PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
    target_link_libraries(optimize-${c_test_name}-c-api-test PRIVATE gecodeoptimize_c)
    add_test(NAME optimize-${c_test_name}-c-api COMMAND optimize-${c_test_name}-c-api-test)
    set_tests_properties(optimize-${c_test_name}-c-api PROPERTIES TIMEOUT 60)
  endforeach()
  find_package(Python3 3.9 COMPONENTS Interpreter QUIET)
  if(Python3_Interpreter_FOUND)
    set(_optimize_containment_arguments)
    if(WIN32)
      list(APPEND _optimize_containment_arguments --require-windows)
    endif()
    add_test(NAME optimize-process-containment COMMAND "${CMAKE_COMMAND}" -E env
      "PYTHONDONTWRITEBYTECODE=1" "${Python3_EXECUTABLE}" -B
      "${GECODE_OPTIMIZE_ROOT}/test/optimize/test_process_containment.py"
      ${_optimize_containment_arguments} -v)
    set_tests_properties(optimize-process-containment PROPERTIES TIMEOUT 60)
  endif()
  if(Python3_Interpreter_FOUND AND GECODE_OPTIMIZE_TEST_PYTHON)
    add_test(NAME optimize-python COMMAND "${CMAKE_COMMAND}" -E env
      "PYTHONDONTWRITEBYTECODE=1" "PYTHONPATH=${GECODE_OPTIMIZE_ROOT}/python"
      "GECODE_OPTIMIZE_LIBRARY=$<TARGET_FILE:gecodeoptimize_c>"
      "${Python3_EXECUTABLE}" -m unittest discover -s "${GECODE_OPTIMIZE_ROOT}/python/tests" -v)
    set_tests_properties(optimize-python PROPERTIES TIMEOUT 60)
  endif()
endif()

add_executable(optimize-example "${GECODE_OPTIMIZE_ROOT}/examples/optimize-linear.cpp")
target_link_libraries(optimize-example PRIVATE gecodeoptimize)
add_executable(optimize-file "${GECODE_OPTIMIZE_ROOT}/examples/optimize-file.cpp")
target_link_libraries(optimize-file PRIVATE gecodeoptimize)

if(GECODE_OPTIMIZE_STANDALONE)
  include(CMakePackageConfigHelpers)
  install(TARGETS gecodeoptimize gecodeoptimize_c EXPORT GecodeOptimizeTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gecode/optimize"
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
    PATTERN "quadratic_bound.hpp" EXCLUDE
    PATTERN "lp_basis_detail.hpp" EXCLUDE
    PATTERN "lp_sensitivity_backend.hpp" EXCLUDE
    PATTERN "lp_sensitivity_highs_detail.hpp" EXCLUDE
    PATTERN "native_regular_limits.hpp" EXCLUDE
    PATTERN "lp_observations_detail.hpp" EXCLUDE)
  install(FILES "${GECODE_OPTIMIZE_ROOT}/gecode/optimize.hh"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gecode")
  # The semantic compiler consumes owning records without a parser dependency.
  install(FILES "${GECODE_OPTIMIZE_ROOT}/gecode/flatzinc/capture-records.hh"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gecode/flatzinc")
  configure_package_config_file(GecodeOptimizeConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/GecodeOptimizeConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/GecodeOptimize")
  write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/GecodeOptimizeConfigVersion.cmake"
    VERSION 0.1.0 COMPATIBILITY SameMajorVersion)
  install(EXPORT GecodeOptimizeTargets NAMESPACE Gecode:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/GecodeOptimize")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GecodeOptimizeConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/GecodeOptimizeConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/GecodeOptimize")
endif()
