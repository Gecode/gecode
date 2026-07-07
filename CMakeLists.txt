#
# CMake build script for Gecode.
#

cmake_minimum_required(VERSION 3.21)

project(GECODE LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/misc/cmake_modules)
include(GNUInstallDirs)

# Match common CMake defaults: tests are opt-in for subprojects.
if(PROJECT_IS_TOP_LEVEL)
  include(CTest)
elseif(NOT DEFINED BUILD_TESTING)
  set(BUILD_TESTING OFF)
endif()

# Keep this preference to preserve existing Gist/OpenGL linkage behavior.
set(OpenGL_GL_PREFERENCE LEGACY)

# ---------------------------------------------------------------------------
# Build options (configure/autoconf parity-oriented)
# ---------------------------------------------------------------------------
# Legacy aliases are translated after option() initialization so they also
# override stale cache values from previous configure runs.
if(DEFINED ENABLE_THREADS)
  set(GECODE_ALIAS_ENABLE_THREAD_VALUE "${ENABLE_THREADS}")
  set(GECODE_USED_ENABLE_THREADS_ALIAS TRUE)
endif()
if(DEFINED ENABLE_GIST)
  set(GECODE_ALIAS_ENABLE_GIST_VALUE "${ENABLE_GIST}")
  set(GECODE_USED_ENABLE_GIST_ALIAS TRUE)
endif()
if(DEFINED BUILD_EXAMPLES)
  set(GECODE_ALIAS_BUILD_EXAMPLES_VALUE "${BUILD_EXAMPLES}")
  set(GECODE_USED_BUILD_EXAMPLES_ALIAS TRUE)
endif()
if(DEFINED ENABLE_CPPROFILER)
  set(GECODE_ALIAS_ENABLE_CPPROFILER_VALUE "${ENABLE_CPPROFILER}")
  set(GECODE_USED_ENABLE_CPPROFILER_ALIAS TRUE)
endif()

set(GECODE_BUILD_SHARED_OR_STATIC_SET_BY_USER FALSE)
if(DEFINED GECODE_BUILD_SHARED OR DEFINED GECODE_BUILD_STATIC)
  set(GECODE_BUILD_SHARED_OR_STATIC_SET_BY_USER TRUE)
endif()

function(gecode_normalize_auto_option option default_value help_text)
  if(DEFINED ${option})
    set(_gecode_auto_option_value "${${option}}")
  else()
    set(_gecode_auto_option_value "${default_value}")
  endif()
  string(TOUPPER "${_gecode_auto_option_value}" _gecode_auto_option_value)
  if(_gecode_auto_option_value STREQUAL "1" OR
     _gecode_auto_option_value STREQUAL "ON" OR
     _gecode_auto_option_value STREQUAL "YES" OR
     _gecode_auto_option_value STREQUAL "TRUE")
    set(_gecode_auto_option_value "ON")
  elseif(_gecode_auto_option_value STREQUAL "0" OR
         _gecode_auto_option_value STREQUAL "OFF" OR
         _gecode_auto_option_value STREQUAL "NO" OR
         _gecode_auto_option_value STREQUAL "FALSE")
    set(_gecode_auto_option_value "OFF")
  elseif(_gecode_auto_option_value STREQUAL "" OR
         _gecode_auto_option_value STREQUAL "AUTO")
    set(_gecode_auto_option_value "AUTO")
  else()
    message(FATAL_ERROR
      "${option} must be AUTO, ON, or OFF; got ${${option}}")
  endif()

  set(${option} "${_gecode_auto_option_value}" CACHE STRING "${help_text}" FORCE)
  set_property(CACHE ${option} PROPERTY STRINGS AUTO ON OFF)
  set(${option} "${_gecode_auto_option_value}" PARENT_SCOPE)
endfunction()

if(NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
endif()
option(BUILD_SHARED_LIBS "Build shared libraries by default" ${BUILD_SHARED_LIBS})

set(GECODE_BUILD_SHARED_DEFAULT ON)
set(GECODE_BUILD_STATIC_DEFAULT OFF)
if(NOT GECODE_BUILD_SHARED_OR_STATIC_SET_BY_USER)
  if(BUILD_SHARED_LIBS)
    set(GECODE_BUILD_SHARED_DEFAULT ON)
    set(GECODE_BUILD_STATIC_DEFAULT OFF)
  else()
    set(GECODE_BUILD_SHARED_DEFAULT OFF)
    set(GECODE_BUILD_STATIC_DEFAULT ON)
  endif()
endif()

option(GECODE_BUILD_SHARED "Build shared libraries" ${GECODE_BUILD_SHARED_DEFAULT})
option(GECODE_BUILD_STATIC "Build static libraries" ${GECODE_BUILD_STATIC_DEFAULT})

option(GECODE_ENABLE_THREAD "Enable thread support" ON)
set(GECODE_ENABLE_QT "AUTO" CACHE STRING "Enable Qt support (AUTO, ON, or OFF)")
set_property(CACHE GECODE_ENABLE_QT PROPERTY STRINGS AUTO ON OFF)
set(GECODE_ENABLE_GIST_DEFAULT AUTO)
if(APPLE AND NOT DEFINED GECODE_ENABLE_GIST AND NOT DEFINED ENABLE_GIST)
  set(GECODE_ENABLE_GIST_DEFAULT OFF)
endif()
set(GECODE_ENABLE_GIST "${GECODE_ENABLE_GIST_DEFAULT}" CACHE STRING "Enable Gist (AUTO, ON, or OFF)")
set_property(CACHE GECODE_ENABLE_GIST PROPERTY STRINGS AUTO ON OFF)
option(GECODE_ENABLE_CPPROFILER "Enable CPProfiler support" ON)
option(GECODE_ENABLE_CBS "Enable counting-based search support" OFF)
set(GECODE_ENABLE_EXAMPLES_DEFAULT ${PROJECT_IS_TOP_LEVEL})
option(GECODE_ENABLE_EXAMPLES "Build examples" ${GECODE_ENABLE_EXAMPLES_DEFAULT})
set(GECODE_INSTALL_DEFAULT ${PROJECT_IS_TOP_LEVEL})
option(GECODE_INSTALL "Enable install/export rules" ${GECODE_INSTALL_DEFAULT})

option(GECODE_ENABLE_SEARCH "Build search module" ON)
option(GECODE_ENABLE_INT_VARS "Build int variables module" ON)
option(GECODE_ENABLE_SET_VARS "Build set variables module" ON)
option(GECODE_ENABLE_FLOAT_VARS "Build float variables module" ON)
option(GECODE_ENABLE_MINIMODEL "Build minimodel module" ON)
option(GECODE_ENABLE_DRIVER "Build driver module" ON)
option(GECODE_ENABLE_FLATZINC "Build FlatZinc module" ON)

option(GECODE_ENABLE_MPFR "Enable MPFR support" ON)
option(GECODE_ENABLE_ALLOCATOR "Enable default allocator" ON)
option(GECODE_ENABLE_AUDIT "Enable audit code" OFF)
option(GECODE_ENABLE_GCC_VISIBILITY "Enable GCC visibility attributes" ON)
option(GECODE_ENABLE_OSX_UNFAIR_MUTEX "Enable macOS unfair mutexes" ON)
option(GECODE_REGENERATE_VARIMP "Regenerate checked-in var-imp headers during build" OFF)
option(GECODE_DOC_DOT "Enable Graphviz dot graphs in Doxygen documentation" ON)
option(GECODE_DOC_SEARCH "Enable the Doxygen documentation search engine" OFF)
option(GECODE_DOC_TAGFILE "Generate the Doxygen tag file" ON)
set(GECODE_FREELIST32_SIZE_MAX "" CACHE STRING "Max freelist size on 32-bit platforms")
set(GECODE_FREELIST64_SIZE_MAX "" CACHE STRING "Max freelist size on 64-bit platforms")
set(GECODE_WITH_VIS "" CACHE STRING "Additional .vis files (comma-separated)")
set(GECODE_LIB_PREFIX "" CACHE STRING "User-defined prefix for generated library basenames")
set(GECODE_LIB_SUFFIX "" CACHE STRING "User-defined suffix for generated library basenames")

# Deprecated compatibility aliases for the Gecode 6.x line.
if(GECODE_USED_ENABLE_THREADS_ALIAS)
  set(GECODE_ENABLE_THREAD ${GECODE_ALIAS_ENABLE_THREAD_VALUE} CACHE BOOL "Enable thread support" FORCE)
  message(WARNING "ENABLE_THREADS is deprecated; use GECODE_ENABLE_THREAD")
endif()
if(GECODE_USED_ENABLE_GIST_ALIAS)
  set(GECODE_ENABLE_GIST ${GECODE_ALIAS_ENABLE_GIST_VALUE} CACHE STRING "Enable Gist (AUTO, ON, or OFF)" FORCE)
  message(WARNING "ENABLE_GIST is deprecated; use GECODE_ENABLE_GIST")
endif()
if(GECODE_USED_BUILD_EXAMPLES_ALIAS)
  set(GECODE_ENABLE_EXAMPLES ${GECODE_ALIAS_BUILD_EXAMPLES_VALUE} CACHE BOOL "Build examples" FORCE)
  message(WARNING "BUILD_EXAMPLES is deprecated; use GECODE_ENABLE_EXAMPLES")
endif()
if(GECODE_USED_ENABLE_CPPROFILER_ALIAS)
  set(GECODE_ENABLE_CPPROFILER ${GECODE_ALIAS_ENABLE_CPPROFILER_VALUE} CACHE BOOL "Enable CPProfiler support" FORCE)
  message(WARNING "ENABLE_CPPROFILER is deprecated; use GECODE_ENABLE_CPPROFILER")
endif()

gecode_normalize_auto_option(GECODE_ENABLE_QT AUTO "Enable Qt support (AUTO, ON, or OFF)")
gecode_normalize_auto_option(GECODE_ENABLE_GIST "${GECODE_ENABLE_GIST_DEFAULT}" "Enable Gist (AUTO, ON, or OFF)")

set(GECODE_ENABLE_QT_REQUIRED FALSE)
set(GECODE_WANT_QT FALSE)
if("${GECODE_ENABLE_QT}" STREQUAL "ON")
  set(GECODE_ENABLE_QT_REQUIRED TRUE)
  set(GECODE_WANT_QT TRUE)
elseif("${GECODE_ENABLE_QT}" STREQUAL "AUTO")
  set(GECODE_WANT_QT TRUE)
endif()

set(GECODE_ENABLE_GIST_REQUIRED FALSE)
set(GECODE_WANT_GIST FALSE)
if("${GECODE_ENABLE_GIST}" STREQUAL "ON")
  set(GECODE_ENABLE_GIST_REQUIRED TRUE)
  set(GECODE_WANT_GIST TRUE)
elseif("${GECODE_ENABLE_GIST}" STREQUAL "AUTO")
  if("${GECODE_ENABLE_QT}" STREQUAL "OFF")
    set(GECODE_WANT_GIST FALSE)
  else()
    set(GECODE_WANT_GIST TRUE)
  endif()
endif()
if(GECODE_ENABLE_GIST_REQUIRED AND "${GECODE_ENABLE_QT}" STREQUAL "OFF")
  message(FATAL_ERROR "GECODE_ENABLE_GIST=ON requires GECODE_ENABLE_QT to be AUTO or ON")
endif()
if(GECODE_WANT_GIST)
  set(GECODE_WANT_QT TRUE)
endif()

if(NOT GECODE_BUILD_SHARED AND NOT GECODE_BUILD_STATIC)
  message(FATAL_ERROR "At least one of GECODE_BUILD_SHARED or GECODE_BUILD_STATIC must be ON")
endif()
if((WIN32 OR CYGWIN) AND GECODE_BUILD_SHARED AND GECODE_BUILD_STATIC)
  message(FATAL_ERROR "Building both shared and static in one tree is not supported on DLL platforms")
endif()

function(gecode_force_option_on option reason)
  if(NOT ${option})
    set(${option} ON CACHE BOOL "${reason}" FORCE)
    message(STATUS "Enabling ${option}: ${reason}")
  endif()
endfunction()

# Keep dependency closure behavior similar to configure.
if(GECODE_ENABLE_SET_VARS)
  gecode_force_option_on(GECODE_ENABLE_INT_VARS "Set variables require int variables")
endif()
if(GECODE_ENABLE_FLOAT_VARS)
  gecode_force_option_on(GECODE_ENABLE_INT_VARS "Float variables require int variables")
endif()
if(GECODE_ENABLE_MINIMODEL)
  gecode_force_option_on(GECODE_ENABLE_SEARCH "Minimodel requires search")
  gecode_force_option_on(GECODE_ENABLE_INT_VARS "Minimodel requires int variables")
  gecode_force_option_on(GECODE_ENABLE_SET_VARS "Minimodel requires set variables")
endif()
if(GECODE_ENABLE_DRIVER)
  gecode_force_option_on(GECODE_ENABLE_SEARCH "Driver requires search")
  gecode_force_option_on(GECODE_ENABLE_INT_VARS "Driver requires int variables")
endif()
if(GECODE_ENABLE_FLATZINC)
  gecode_force_option_on(GECODE_ENABLE_SEARCH "FlatZinc requires search")
  gecode_force_option_on(GECODE_ENABLE_INT_VARS "FlatZinc requires int variables")
  gecode_force_option_on(GECODE_ENABLE_SET_VARS "FlatZinc requires set variables")
  gecode_force_option_on(GECODE_ENABLE_FLOAT_VARS "FlatZinc requires float variables")
  gecode_force_option_on(GECODE_ENABLE_MINIMODEL "FlatZinc requires minimodel")
  gecode_force_option_on(GECODE_ENABLE_DRIVER "FlatZinc requires driver")
endif()
if(GECODE_ENABLE_EXAMPLES)
  gecode_force_option_on(GECODE_ENABLE_SEARCH "Examples require search")
  gecode_force_option_on(GECODE_ENABLE_INT_VARS "Examples require int variables")
  gecode_force_option_on(GECODE_ENABLE_SET_VARS "Examples require set variables")
  gecode_force_option_on(GECODE_ENABLE_FLOAT_VARS "Examples require float variables")
  gecode_force_option_on(GECODE_ENABLE_MINIMODEL "Examples require minimodel")
  gecode_force_option_on(GECODE_ENABLE_DRIVER "Examples require driver")
endif()

include(CheckCXXCompilerFlag)
set(GECODE_VISIBILITY_COMPILE_OPTION)
if(GECODE_ENABLE_GCC_VISIBILITY)
  check_cxx_compiler_flag(-fvisibility=hidden HAVE_VISIBILITY_HIDDEN_FLAG)
  if(HAVE_VISIBILITY_HIDDEN_FLAG)
    set(GECODE_VISIBILITY_COMPILE_OPTION -fvisibility=hidden)
  endif()
endif()

set(GECODE_QT_MIN_VERSION "5.15")

if(GECODE_REGENERATE_VARIMP)
  find_program(GECODE_UV_EXECUTABLE NAMES uv)
  if(NOT GECODE_UV_EXECUTABLE)
    message(FATAL_ERROR "GECODE_REGENERATE_VARIMP=ON requires uv on PATH")
  endif()
endif()

function(gecode_collect_defined_values out_var)
  set(_gecode_values)
  foreach(_gecode_var ${ARGN})
    if(DEFINED ${_gecode_var})
      list(APPEND _gecode_values ${${_gecode_var}})
    endif()
  endforeach()
  if(_gecode_values)
    list(REMOVE_DUPLICATES _gecode_values)
  endif()
  set(${out_var} ${_gecode_values} PARENT_SCOPE)
endfunction()

function(gecode_find_named_library out_var library_name)
  set(_gecode_result)
  foreach(_gecode_library ${ARGN})
    get_filename_component(_gecode_library_name "${_gecode_library}" NAME)
    if(_gecode_library STREQUAL "-l${library_name}" OR
       _gecode_library_name MATCHES "(^|lib)${library_name}([._-]|$)")
      set(_gecode_result "${_gecode_library}")
      break()
    endif()
  endforeach()
  set(${out_var} "${_gecode_result}" PARENT_SCOPE)
endfunction()

function(gecode_imported_location_from_library out_var library)
  set(_gecode_location)
  if(NOT "${library}" STREQUAL "")
    if(IS_ABSOLUTE "${library}" OR EXISTS "${library}")
      set(_gecode_location "${library}")
    endif()
  endif()
  set(${out_var} "${_gecode_location}" PARENT_SCOPE)
endfunction()

function(gecode_normalize_mpfr_targets)
  if(NOT MPFR_FOUND)
    return()
  endif()

  gecode_collect_defined_values(_gecode_mpfr_include_dirs
    MPFR_INCLUDE_DIRS MPFR_INCLUDES MPFR_INCLUDE_DIR)
  gecode_collect_defined_values(_gecode_gmp_include_dirs
    GMP_INCLUDE_DIRS GMP_INCLUDES GMP_INCLUDE_DIR)
  gecode_collect_defined_values(_gecode_mpfr_libraries
    MPFR_LIBRARIES MPFR_LIBRARY)
  gecode_collect_defined_values(_gecode_gmp_libraries
    GMP_LIBRARIES GMP_LIBRARY)

  gecode_find_named_library(_gecode_mpfr_library mpfr
    ${_gecode_mpfr_libraries})
  gecode_find_named_library(_gecode_gmp_library gmp
    ${_gecode_gmp_libraries} ${_gecode_mpfr_libraries})
  gecode_imported_location_from_library(_gecode_mpfr_location
    "${_gecode_mpfr_library}")
  gecode_imported_location_from_library(_gecode_gmp_location
    "${_gecode_gmp_library}")

  if(NOT TARGET GMP::GMP AND
     (_gecode_gmp_library OR _gecode_gmp_libraries OR _gecode_gmp_include_dirs))
    if(_gecode_gmp_location)
      add_library(GMP::GMP UNKNOWN IMPORTED)
      set_target_properties(GMP::GMP PROPERTIES
        IMPORTED_LOCATION "${_gecode_gmp_location}")
    else()
      add_library(GMP::GMP INTERFACE IMPORTED)
      set(_gecode_gmp_link_libraries ${_gecode_gmp_libraries})
      if(_gecode_gmp_library AND NOT _gecode_gmp_library IN_LIST _gecode_gmp_link_libraries)
        list(APPEND _gecode_gmp_link_libraries "${_gecode_gmp_library}")
      endif()
      if(_gecode_gmp_link_libraries)
        set_target_properties(GMP::GMP PROPERTIES
          INTERFACE_LINK_LIBRARIES "${_gecode_gmp_link_libraries}")
      endif()
    endif()
    if(_gecode_gmp_include_dirs)
      set_target_properties(GMP::GMP PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_gecode_gmp_include_dirs}")
    endif()
  endif()

  if(NOT TARGET MPFR::MPFR)
    if(NOT _gecode_mpfr_library AND NOT _gecode_mpfr_libraries)
      message(FATAL_ERROR
        "MPFR was found without an MPFR::MPFR target or MPFR_LIBRARIES/MPFR_LIBRARY; "
        "cannot enable MPFR support safely.")
    endif()

    set(_gecode_mpfr_link_libraries)
    if(TARGET GMP::GMP)
      list(APPEND _gecode_mpfr_link_libraries GMP::GMP)
    elseif(_gecode_gmp_libraries)
      list(APPEND _gecode_mpfr_link_libraries ${_gecode_gmp_libraries})
    endif()

    set(_gecode_mpfr_target_libraries ${_gecode_mpfr_libraries})
    if(TARGET GMP::GMP AND _gecode_gmp_library)
      list(REMOVE_ITEM _gecode_mpfr_target_libraries "${_gecode_gmp_library}")
    endif()

    if(_gecode_mpfr_location)
      add_library(MPFR::MPFR UNKNOWN IMPORTED)
      set_target_properties(MPFR::MPFR PROPERTIES
        IMPORTED_LOCATION "${_gecode_mpfr_location}")
      if(_gecode_mpfr_library)
        list(REMOVE_ITEM _gecode_mpfr_target_libraries "${_gecode_mpfr_library}")
      endif()
    else()
      add_library(MPFR::MPFR INTERFACE IMPORTED)
    endif()
    list(APPEND _gecode_mpfr_link_libraries ${_gecode_mpfr_target_libraries})
    if(_gecode_mpfr_include_dirs)
      set_target_properties(MPFR::MPFR PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_gecode_mpfr_include_dirs}")
    endif()
    if(_gecode_mpfr_link_libraries)
      set_target_properties(MPFR::MPFR PROPERTIES
        INTERFACE_LINK_LIBRARIES "${_gecode_mpfr_link_libraries}")
    endif()
  endif()
endfunction()

if(GECODE_ENABLE_MPFR)
  find_package(MPFR)
  if(NOT MPFR_FOUND)
    message(WARNING
      "GECODE_ENABLE_MPFR=ON but MPFR was not found; float-MPFR support will be disabled. "
      "Set CMAKE_PREFIX_PATH, MPFR_ROOT, or a toolchain file (for example vcpkg) to locate MPFR.")
  else()
    gecode_normalize_mpfr_targets()
  endif()
endif()

set(GECODE_HAS_QT_RUNTIME FALSE)
set(GECODE_HAS_QT_WIDGETS FALSE)
set(GECODE_QT_MAJOR)
set(GECODE_QT_CORE)
set(GECODE_QT_WIDGETS)
set(GECODE_QT_VERSION)
if(GECODE_WANT_QT)
  set(_gecode_qt_runtime_candidate_major)
  set(_gecode_qt_runtime_candidate_version)
  set(_gecode_qt_widgets_candidate_major)
  set(_gecode_qt_widgets_candidate_version)
  set(_gecode_qt_old_version)

  foreach(_gecode_qt_major 6 5)
    find_package(Qt${_gecode_qt_major} QUIET COMPONENTS Core
                 OPTIONAL_COMPONENTS Gui Widgets PrintSupport)
    if(NOT TARGET Qt${_gecode_qt_major}::Core)
      continue()
    endif()

    if(_gecode_qt_major EQUAL 6)
      set(_gecode_qt_version "${Qt6Core_VERSION}")
    else()
      set(_gecode_qt_version "${Qt5Core_VERSION}")
    endif()

    if(_gecode_qt_version VERSION_LESS GECODE_QT_MIN_VERSION)
      if(NOT _gecode_qt_old_version)
        set(_gecode_qt_old_version "${_gecode_qt_version}")
      endif()
      continue()
    endif()

    if(NOT _gecode_qt_runtime_candidate_major)
      set(_gecode_qt_runtime_candidate_major "${_gecode_qt_major}")
      set(_gecode_qt_runtime_candidate_version "${_gecode_qt_version}")
    endif()

    set(_gecode_qt_has_widgets TRUE)
    foreach(_gecode_qt_target
        Qt${_gecode_qt_major}::Widgets
        Qt${_gecode_qt_major}::Gui
        Qt${_gecode_qt_major}::PrintSupport)
      if(NOT TARGET ${_gecode_qt_target})
        set(_gecode_qt_has_widgets FALSE)
      endif()
    endforeach()
    if(_gecode_qt_has_widgets AND NOT _gecode_qt_widgets_candidate_major)
      set(_gecode_qt_widgets_candidate_major "${_gecode_qt_major}")
      set(_gecode_qt_widgets_candidate_version "${_gecode_qt_version}")
    endif()
  endforeach()

  if(GECODE_WANT_GIST AND _gecode_qt_widgets_candidate_major)
    set(GECODE_QT_MAJOR "${_gecode_qt_widgets_candidate_major}")
    set(GECODE_QT_VERSION "${_gecode_qt_widgets_candidate_version}")
  elseif(_gecode_qt_runtime_candidate_major)
    set(GECODE_QT_MAJOR "${_gecode_qt_runtime_candidate_major}")
    set(GECODE_QT_VERSION "${_gecode_qt_runtime_candidate_version}")
  endif()

  if(GECODE_QT_MAJOR)
    set(GECODE_HAS_QT_RUNTIME TRUE)
    set(GECODE_QT_CORE Qt${GECODE_QT_MAJOR}::Core)
    set(GECODE_QT_WIDGETS
      Qt${GECODE_QT_MAJOR}::Widgets
      Qt${GECODE_QT_MAJOR}::Gui
      Qt${GECODE_QT_MAJOR}::PrintSupport)
    set(GECODE_HAS_QT_WIDGETS TRUE)
    foreach(qt_target ${GECODE_QT_WIDGETS})
      if(NOT TARGET ${qt_target})
        set(GECODE_HAS_QT_WIDGETS FALSE)
      endif()
    endforeach()
  elseif(GECODE_ENABLE_QT_REQUIRED)
    if(_gecode_qt_old_version)
      message(FATAL_ERROR
        "GECODE_ENABLE_QT=ON requires Qt ${GECODE_QT_MIN_VERSION} or newer; found ${_gecode_qt_old_version}")
    endif()
    message(FATAL_ERROR "GECODE_ENABLE_QT=ON requires Qt ${GECODE_QT_MIN_VERSION} or newer")
  elseif(_gecode_qt_old_version)
    message(WARNING
      "Qt ${_gecode_qt_old_version} was found, but Gecode requires Qt ${GECODE_QT_MIN_VERSION} or newer. "
      "Disabling Qt support.")
  endif()

  unset(_gecode_qt_has_widgets)
  unset(_gecode_qt_major)
  unset(_gecode_qt_old_version)
  unset(_gecode_qt_runtime_candidate_major)
  unset(_gecode_qt_runtime_candidate_version)
  unset(_gecode_qt_target)
  unset(_gecode_qt_version)
  unset(_gecode_qt_widgets_candidate_major)
  unset(_gecode_qt_widgets_candidate_version)
endif()

set(GECODE_BUILD_GIST_TARGET ${GECODE_WANT_GIST})
if(GECODE_BUILD_GIST_TARGET AND NOT GECODE_HAS_QT_WIDGETS)
  if(GECODE_ENABLE_GIST_REQUIRED)
    message(FATAL_ERROR "GECODE_ENABLE_GIST=ON requires Qt Core, Gui, Widgets, and PrintSupport")
  endif()
  message(STATUS "Gist will not be built because the required Qt GUI components were not found")
  set(GECODE_BUILD_GIST_TARGET FALSE)
endif()
if(GECODE_BUILD_GIST_TARGET)
  include(CheckCXXSourceCompiles)
  set(_gecode_saved_required_libs "${CMAKE_REQUIRED_LIBRARIES}")
  set(CMAKE_REQUIRED_LIBRARIES ${GECODE_QT_WIDGETS})
  check_cxx_source_compiles(
    "#include <QtWidgets/QApplication>
int main(int argc, char** argv) {
  QApplication app(argc, argv);
  return 0;
}
"
    GECODE_GIST_QT_LINKS)
  set(CMAKE_REQUIRED_LIBRARIES "${_gecode_saved_required_libs}")
  unset(_gecode_saved_required_libs)
  if(NOT GECODE_GIST_QT_LINKS)
    if(GECODE_ENABLE_GIST_REQUIRED)
      message(FATAL_ERROR "GECODE_ENABLE_GIST=ON but Qt/Gist link dependencies are not usable")
    endif()
    message(STATUS "Gist will not be built because the Qt/Gist link dependencies are not usable")
    set(GECODE_BUILD_GIST_TARGET FALSE)
  endif()
endif()
if(GECODE_BUILD_GIST_TARGET)
  gecode_force_option_on(GECODE_ENABLE_SEARCH "Gist requires search")
  gecode_force_option_on(GECODE_ENABLE_INT_VARS "Gist requires int variables")
endif()

# ---------------------------------------------------------------------------
# Configure-time metadata and config.hpp generation
# ---------------------------------------------------------------------------
file(READ gecode/support/config.hpp.in CONFIG)
string(REGEX REPLACE "^/\\*([^*]|\\*[^/])*\\*/" "" CONFIG ${CONFIG})
string(REGEX MATCHALL "[^\n]*\n" CONFIG
"${CONFIG}
/* Define to 1 if you have a working `mmap' system call. */
#undef HAVE_MMAP

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#undef PACKAGE_VERSION
")

set(GECODE_VERSION_METADATA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/gecode-version.m4")
if(NOT EXISTS "${GECODE_VERSION_METADATA_FILE}")
  message(FATAL_ERROR "Missing version metadata file: ${GECODE_VERSION_METADATA_FILE}")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${GECODE_VERSION_METADATA_FILE}")
file(READ "${GECODE_VERSION_METADATA_FILE}" GECODE_VERSION_METADATA)

function(gecode_extract_version_metadata macro_name output_var)
  string(REGEX MATCH "m4_define\\(\\[${macro_name}\\], *\\[([^]]+)\\]\\)" _match
    "${GECODE_VERSION_METADATA}")
  if(NOT _match)
    message(FATAL_ERROR
      "Could not parse ${macro_name} from ${GECODE_VERSION_METADATA_FILE}")
  endif()
  set(${output_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()

gecode_extract_version_metadata("GECODE_M4_VERSION" VERSION)
gecode_extract_version_metadata("GECODE_M4_SOVERSION" GECODE_SOVERSION)

set(PACKAGE "GECODE")
set(PACKAGE_BUGREPORT "users@gecode.dev")

set(PACKAGE_NAME ${PACKAGE})
string(TOLOWER ${PACKAGE} PACKAGE_TARNAME)
set(PACKAGE_URL "")
set(PACKAGE_VERSION ${VERSION})
set(${PACKAGE}_VERSION ${VERSION})
set(GECODE_PROJECT_VERSION ${VERSION})
string(REPLACE "." "-" GECODE_LIBRARY_VERSION "${VERSION}")
set(PACKAGE_STRING "${PACKAGE} ${VERSION}")
if(VERSION MATCHES "(.*)\\.(.*)\\.(.*)")
  math(EXPR GECODE_VERSION_NUMBER
    "${CMAKE_MATCH_1} * 100000 + ${CMAKE_MATCH_2} * 100 + ${CMAKE_MATCH_3}")
endif()

set(GECODE_DLL_USERPREFIX "${GECODE_LIB_PREFIX}")
set(GECODE_DLL_USERSUFFIX "${GECODE_LIB_SUFFIX}")
if(GECODE_ENABLE_INT_VARS)
  set(GECODE_HAS_INT_VARS "/**/")
endif()
if(GECODE_ENABLE_SET_VARS)
  set(GECODE_HAS_SET_VARS "/**/")
endif()
if(GECODE_ENABLE_FLOAT_VARS)
  set(GECODE_HAS_FLOAT_VARS "/**/")
endif()
if(GECODE_BUILD_STATIC)
  set(GECODE_STATIC_LIBS 1)
endif()
if(GECODE_ENABLE_ALLOCATOR)
  set(GECODE_ALLOCATOR "/**/")
endif()
if(GECODE_ENABLE_AUDIT)
  set(GECODE_AUDIT "/**/")
endif()
if(GECODE_ENABLE_CPPROFILER)
  set(GECODE_HAS_CPPROFILER "/**/")
endif()
if(GECODE_ENABLE_CBS)
  set(GECODE_HAS_CBS "/**/")
endif()
if(GECODE_ENABLE_MPFR AND MPFR_FOUND AND GECODE_ENABLE_FLOAT_VARS)
  set(GECODE_HAS_MPFR "/**/")
endif()
if(HAVE_VISIBILITY_HIDDEN_FLAG)
  set(GECODE_GCC_HAS_CLASS_VISIBILITY "/**/")
endif()
if(GECODE_ENABLE_THREAD)
  set(GECODE_HAS_THREADS 1)
endif()
if(APPLE AND GECODE_ENABLE_THREAD AND GECODE_ENABLE_OSX_UNFAIR_MUTEX)
  set(GECODE_USE_OSX_UNFAIR_MUTEX "/**/")
endif()
if(GECODE_HAS_QT_RUNTIME)
  set(GECODE_HAS_QT "/**/")
endif()
if(GECODE_BUILD_GIST_TARGET)
  set(GECODE_HAS_GIST "/**/")
endif()
if(GECODE_FREELIST32_SIZE_MAX)
  set(GECODE_FREELIST_SIZE_MAX32 ${GECODE_FREELIST32_SIZE_MAX})
endif()
if(GECODE_FREELIST64_SIZE_MAX)
  set(GECODE_FREELIST_SIZE_MAX64 ${GECODE_FREELIST64_SIZE_MAX})
endif()

include(CheckSymbolExists)
check_symbol_exists(mmap sys/mman.h HAVE_MMAP)

include(CheckCSourceCompiles)
check_c_source_compiles("inline __attribute__ ((__always_inline__)) void foo(void) {}\nint main() {}" HAVE_ALWAYS_INLINE)
check_c_source_compiles("__forceinline void foo(void) {}\nint main() {}" HAVE_FORCE_INLINE)
set(forceinline inline)
if(HAVE_ALWAYS_INLINE)
  set(forceinline "inline __attribute__ ((__always_inline__))")
endif()
if(HAVE_FORCE_INLINE)
  set(forceinline "__forceinline")
endif()

check_c_source_compiles("int main() { return __builtin_ffsll(0); }" HAVE_BUILTIN_FFSLL)
if(HAVE_BUILTIN_FFSLL)
  set(GECODE_HAS_BUILTIN_FFSLL "/**/")
endif()
check_c_source_compiles("int main() { return __builtin_popcountll(0); }" HAVE_BUILTIN_POPCOUNTLL)
if(HAVE_BUILTIN_POPCOUNTLL)
  set(GECODE_HAS_BUILTIN_POPCOUNTLL "/**/")
endif()

# Process config.hpp using autoconf-like undef handling.
list(LENGTH CONFIG length)
math(EXPR length "${length} - 1")
foreach(i RANGE ${length})
  list(GET CONFIG ${i} line)
  if(line MATCHES "^#( *)undef (.*)\n")
    set(space "${CMAKE_MATCH_1}")
    set(var ${CMAKE_MATCH_2})
    if(NOT DEFINED ${var} OR (var MATCHES "HAVE_.*" AND NOT ${var}))
      set(line "/* #${space}undef ${var} */\n")
    else()
      if("${${var}}" STREQUAL "/**/" OR "${var}" STREQUAL "GECODE_VERSION_NUMBER" OR
         "${var}" STREQUAL "forceinline" OR var MATCHES "SIZEOF_.*")
        set(value ${${var}})
      elseif(NOT (var MATCHES ^HAVE OR ${var} EQUAL 0 OR ${var} EQUAL 1))
        set(value \"${${var}}\")
      elseif(${var})
        set(value 1)
      else()
        set(value 0)
      endif()
      set(line "#${space}define ${var} ${value}\n")
    endif()
  endif()
  string(REPLACE "<semi>" ";" line "${line}")
  set(CONFIG_OUT "${CONFIG_OUT}${line}")
endforeach()

file(WRITE ${GECODE_BINARY_DIR}/gecode/support/config.hpp
"/* gecode/support/config.hpp.  Generated from config.hpp.in by configure.  */
/* gecode/support/config.hpp.in.  Generated from configure.ac by autoheader.  */

${CONFIG_OUT}")

# ---------------------------------------------------------------------------
# Native source inventory for CMake (decoupled from Makefile.in).
# ---------------------------------------------------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GecodeSources.cmake)

# ---------------------------------------------------------------------------
# Generated variable implementations (in-source, matching Makefile behavior)
# ---------------------------------------------------------------------------
set(GECODE_VIS_FILES)
if(GECODE_ENABLE_INT_VARS)
  list(APPEND GECODE_VIS_FILES
    gecode/int/var-imp/int.vis
    gecode/int/var-imp/bool.vis)
endif()
if(GECODE_ENABLE_SET_VARS)
  list(APPEND GECODE_VIS_FILES
    gecode/set/var-imp/set.vis)
endif()
if(GECODE_ENABLE_FLOAT_VARS)
  list(APPEND GECODE_VIS_FILES
    gecode/float/var-imp/float.vis)
endif()
if(GECODE_WITH_VIS)
  string(REPLACE "," ";" _extra_vis "${GECODE_WITH_VIS}")
  foreach(vis ${_extra_vis})
    list(APPEND GECODE_VIS_FILES "${vis}")
  endforeach()
endif()
list(REMOVE_DUPLICATES GECODE_VIS_FILES)
if(GECODE_REGENERATE_VARIMP AND NOT GECODE_VIS_FILES)
  message(FATAL_ERROR
    "GECODE_REGENERATE_VARIMP=ON requires at least one enabled variable module "
    "or an explicit GECODE_WITH_VIS list")
endif()

set(GECODE_VIS_DEPENDS)
set(GECODE_VIS_FILES_FOR_GEN)
foreach(vis ${GECODE_VIS_FILES})
  if(IS_ABSOLUTE "${vis}")
    list(APPEND GECODE_VIS_DEPENDS "${vis}")
    list(APPEND GECODE_VIS_FILES_FOR_GEN "${vis}")
  else()
    list(APPEND GECODE_VIS_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${vis}")
    if(vis MATCHES "^\\./")
      list(APPEND GECODE_VIS_FILES_FOR_GEN "${vis}")
    else()
      list(APPEND GECODE_VIS_FILES_FOR_GEN "./${vis}")
    endif()
  endif()
endforeach()
string(JOIN "||" GECODE_VIS_FILES_SERIALIZED ${GECODE_VIS_FILES_FOR_GEN})

set(GECODE_VAR_TYPE_HPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/gecode/kernel/var-type.hpp)
set(GECODE_VAR_IMP_HPP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/gecode/kernel/var-imp.hpp)
set(GECODE_VAR_TYPE_HPP ${GECODE_VAR_TYPE_HPP_SOURCE})
set(GECODE_VAR_IMP_HPP ${GECODE_VAR_IMP_HPP_SOURCE})

if(GECODE_REGENERATE_VARIMP)
  set(GECODE_VAR_TYPE_HPP ${CMAKE_CURRENT_BINARY_DIR}/gecode/kernel/var-type.hpp)
  set(GECODE_VAR_IMP_HPP ${CMAKE_CURRENT_BINARY_DIR}/gecode/kernel/var-imp.hpp)
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gecode/kernel")

  add_custom_command(
    OUTPUT ${GECODE_VAR_TYPE_HPP}
    COMMAND ${CMAKE_COMMAND}
      -DUV_EXECUTABLE=${GECODE_UV_EXECUTABLE}
      -DGENVARIMP=${CMAKE_CURRENT_SOURCE_DIR}/misc/genvarimp.py
      -DMODE=-typehpp
      -DOUT_FILE=${GECODE_VAR_TYPE_HPP}
      -DVIS_FILES_SERIALIZED=${GECODE_VIS_FILES_SERIALIZED}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVarImp.cmake
    DEPENDS ${GECODE_VIS_DEPENDS} ${CMAKE_CURRENT_SOURCE_DIR}/misc/genvarimp.py ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt ${CMAKE_CURRENT_SOURCE_DIR}/configure.ac
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
  )
  add_custom_command(
    OUTPUT ${GECODE_VAR_IMP_HPP}
    COMMAND ${CMAKE_COMMAND}
      -DUV_EXECUTABLE=${GECODE_UV_EXECUTABLE}
      -DGENVARIMP=${CMAKE_CURRENT_SOURCE_DIR}/misc/genvarimp.py
      -DMODE=-header
      -DOUT_FILE=${GECODE_VAR_IMP_HPP}
      -DVIS_FILES_SERIALIZED=${GECODE_VIS_FILES_SERIALIZED}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVarImp.cmake
    DEPENDS ${GECODE_VIS_DEPENDS} ${CMAKE_CURRENT_SOURCE_DIR}/misc/genvarimp.py ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt ${CMAKE_CURRENT_SOURCE_DIR}/configure.ac
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
  )
  add_custom_target(gecode-varimp-gen DEPENDS ${GECODE_VAR_TYPE_HPP} ${GECODE_VAR_IMP_HPP})
else()
  add_custom_target(gecode-varimp-gen)
endif()

# ---------------------------------------------------------------------------
# Doxygen documentation
# ---------------------------------------------------------------------------
set(GECODE_DOXYGEN_MIN_VERSION 1.17.0)
find_package(Doxygen ${GECODE_DOXYGEN_MIN_VERSION} QUIET)
find_program(GECODE_DOT_EXECUTABLE NAMES dot)

set(GECODE_DOXYGEN_DOT NO)
if(GECODE_DOC_DOT AND GECODE_DOT_EXECUTABLE)
  set(GECODE_DOXYGEN_DOT YES)
endif()

set(GECODE_DOXYGEN_TAGFILE)
if(GECODE_DOC_TAGFILE)
  set(GECODE_DOXYGEN_TAGFILE "doc/gecode-doc.tag")
endif()

set(top_srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
set(abs_top_srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen/doxygen.hh.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.hh @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen/doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf @ONLY)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/misc/doxygen/header.html GECODE_DOXYGEN_HEADER)
if(NOT GECODE_DOC_SEARCH)
  string(REGEX REPLACE "<form[^>\n]*>[^<]*<a[^>]*>[^<]*<input[^>]*/>[^<]*</a></form>" ""
         GECODE_DOXYGEN_HEADER "${GECODE_DOXYGEN_HEADER}")
endif()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/header.html "${GECODE_DOXYGEN_HEADER}")

set(GECODE_DOXYGEN_SEARCH NO)
set(GECODE_DOXYGEN_SERVER_SEARCH NO)
if(GECODE_DOC_SEARCH)
  set(GECODE_DOXYGEN_SEARCH YES)
  set(GECODE_DOXYGEN_SERVER_SEARCH YES)
endif()
file(READ ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf GECODE_DOXYGEN_CONFIG)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf.use
"${GECODE_DOXYGEN_CONFIG}
SEARCHENGINE = ${GECODE_DOXYGEN_SEARCH}
SERVER_BASED_SEARCH = ${GECODE_DOXYGEN_SERVER_SEARCH}
HAVE_DOT = ${GECODE_DOXYGEN_DOT}
")

set(GECODE_DOC_SOURCE_INPUTS)
foreach(component SUPPORT KERNEL SEARCH INT SET FLOAT MINIMODEL DRIVER GIST FLATZINC TEST)
  list(APPEND GECODE_DOC_SOURCE_INPUTS ${GECODE_${component}_SOURCES})
endforeach()
list(APPEND GECODE_DOC_SOURCE_INPUTS ${GECODE_FLATZINC_EXE_SOURCE})

file(GLOB_RECURSE GECODE_DOC_HEADER_INPUTS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/gecode/*.hh"
  "${CMAKE_CURRENT_SOURCE_DIR}/gecode/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/*.hh"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp")
file(GLOB GECODE_DOC_EXAMPLE_INPUTS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.hh"
  "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.hpp")

set(GECODE_DOC_HELPER_INPUTS ${GECODE_DOC_HEADER_INPUTS} ${GECODE_DOC_EXAMPLE_INPUTS})
foreach(input ${GECODE_DOC_SOURCE_INPUTS})
  if(input)
    if(IS_ABSOLUTE "${input}")
      list(APPEND GECODE_DOC_HELPER_INPUTS "${input}")
    else()
      list(APPEND GECODE_DOC_HELPER_INPUTS "${CMAKE_CURRENT_SOURCE_DIR}/${input}")
    endif()
  endif()
endforeach()
list(APPEND GECODE_DOC_HELPER_INPUTS ${GECODE_VAR_TYPE_HPP} ${GECODE_VAR_IMP_HPP})
list(FILTER GECODE_DOC_HELPER_INPUTS EXCLUDE REGEX "/gecode/(third-party|gist/standalone-example|flatzinc/exampleplugin)/")
list(FILTER GECODE_DOC_HELPER_INPUTS EXCLUDE REGEX "/gecode/flatzinc/(parser\\.tab|lexer\\.)")
list(REMOVE_DUPLICATES GECODE_DOC_HELPER_INPUTS)

set(GECODE_DOC_HELPER_INPUT_LIST ${CMAKE_CURRENT_BINARY_DIR}/doxygen-helper-inputs.txt)
file(WRITE ${GECODE_DOC_HELPER_INPUT_LIST} "")
foreach(input ${GECODE_DOC_HELPER_INPUTS})
  file(APPEND ${GECODE_DOC_HELPER_INPUT_LIST} "${input}\n")
endforeach()

if(NOT GECODE_UV_EXECUTABLE)
  find_program(GECODE_UV_EXECUTABLE NAMES uv)
endif()

set(GECODE_DOXYGEN_HELPER ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunDoxygenGenerator.cmake)
set(GECODE_DOXYGEN_COMPAT_ALIASES
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateDoxygenCompatibilityAliases.cmake)
set(GECODE_DOXYGEN_GENERATED_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/changelog.hh
  ${CMAKE_CURRENT_BINARY_DIR}/license.hh
  ${CMAKE_CURRENT_BINARY_DIR}/stat.hh)

if(DOXYGEN_FOUND AND GECODE_UV_EXECUTABLE)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/changelog.hh
    COMMAND ${CMAKE_COMMAND}
      -DUV_EXECUTABLE=${GECODE_UV_EXECUTABLE}
      -DSCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/misc/genchangelog.py
      -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/changelog.in
      -DOUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/changelog.hh
      -P ${GECODE_DOXYGEN_HELPER}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/misc/genchangelog.py
            ${CMAKE_CURRENT_SOURCE_DIR}/changelog.in
            ${GECODE_DOXYGEN_HELPER}
    VERBATIM)

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/license.hh
    COMMAND ${CMAKE_COMMAND}
      -DUV_EXECUTABLE=${GECODE_UV_EXECUTABLE}
      -DSCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/misc/genlicense.py
      -DINPUT_LIST=${GECODE_DOC_HELPER_INPUT_LIST}
      -DOUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/license.hh
      -P ${GECODE_DOXYGEN_HELPER}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/misc/genlicense.py
            ${GECODE_DOC_HELPER_INPUT_LIST}
            ${GECODE_DOC_HELPER_INPUTS}
            ${GECODE_DOXYGEN_HELPER}
    VERBATIM)

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stat.hh
    COMMAND ${CMAKE_COMMAND}
      -DUV_EXECUTABLE=${GECODE_UV_EXECUTABLE}
      -DSCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/misc/genstatistics.py
      -DINPUT_LIST=${GECODE_DOC_HELPER_INPUT_LIST}
      -DOUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/stat.hh
      -P ${GECODE_DOXYGEN_HELPER}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/misc/genstatistics.py
            ${GECODE_DOC_HELPER_INPUT_LIST}
            ${GECODE_DOC_HELPER_INPUTS}
            ${GECODE_DOXYGEN_HELPER}
    VERBATIM)

  add_custom_target(doc
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/doc/html
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/doc/html
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/misc/doxygen/back.png
            ${CMAKE_CURRENT_SOURCE_DIR}/misc/doxygen/gecode-logo-100.png
            ${CMAKE_CURRENT_BINARY_DIR}/doc/html
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf.use
    COMMAND ${CMAKE_COMMAND}
            "-DGECODE_DOXYGEN_HTML_DIR=${CMAKE_CURRENT_BINARY_DIR}/doc/html"
            -P ${GECODE_DOXYGEN_COMPAT_ALIASES}
    DEPENDS gecode-varimp-gen
            ${CMAKE_CURRENT_BINARY_DIR}/doxygen.hh
            ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf.use
            ${CMAKE_CURRENT_BINARY_DIR}/header.html
            ${CMAKE_CURRENT_SOURCE_DIR}/misc/doxygen/footer.html
            ${CMAKE_CURRENT_SOURCE_DIR}/misc/doxygen/stylesheet.css
            ${GECODE_DOXYGEN_COMPAT_ALIASES}
            ${GECODE_DOXYGEN_GENERATED_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL)
else()
  set(GECODE_DOC_MISSING_TOOLS)
  if(NOT DOXYGEN_FOUND)
    list(APPEND GECODE_DOC_MISSING_TOOLS "doxygen >= ${GECODE_DOXYGEN_MIN_VERSION}")
  endif()
  if(NOT GECODE_UV_EXECUTABLE)
    list(APPEND GECODE_DOC_MISSING_TOOLS "uv")
  endif()
  string(JOIN ", " GECODE_DOC_MISSING_TOOLS_TEXT ${GECODE_DOC_MISSING_TOOLS})
  add_custom_target(doc
    COMMAND ${CMAKE_COMMAND} -E echo "Cannot build documentation; missing: ${GECODE_DOC_MISSING_TOOLS_TEXT}"
    COMMAND ${CMAKE_COMMAND} -E false)
endif()

# ---------------------------------------------------------------------------
# Target creation helpers
# ---------------------------------------------------------------------------
set(GECODE_LIBRARY_COMPONENTS support kernel)
if(GECODE_ENABLE_SEARCH)
  list(APPEND GECODE_LIBRARY_COMPONENTS search)
endif()
if(GECODE_ENABLE_INT_VARS)
  list(APPEND GECODE_LIBRARY_COMPONENTS int)
endif()
if(GECODE_ENABLE_SET_VARS)
  list(APPEND GECODE_LIBRARY_COMPONENTS set)
endif()
if(GECODE_ENABLE_FLOAT_VARS)
  list(APPEND GECODE_LIBRARY_COMPONENTS float)
endif()
if(GECODE_ENABLE_MINIMODEL)
  list(APPEND GECODE_LIBRARY_COMPONENTS minimodel)
endif()
if(GECODE_ENABLE_DRIVER)
  list(APPEND GECODE_LIBRARY_COMPONENTS driver)
endif()
if(GECODE_ENABLE_FLATZINC)
  list(APPEND GECODE_LIBRARY_COMPONENTS flatzinc)
endif()
if(GECODE_BUILD_GIST_TARGET)
  list(APPEND GECODE_LIBRARY_COMPONENTS gist)
endif()

set(GECODE_DEFAULT_LINK_VARIANT)
if(GECODE_BUILD_SHARED)
  set(GECODE_DEFAULT_LINK_VARIANT shared)
else()
  set(GECODE_DEFAULT_LINK_VARIANT static)
endif()

function(add_gecode_component_library lib)
  string(TOUPPER ${lib} libupper)
  set(sources ${GECODE_${libupper}_SOURCES})
  set(component_output_name "${GECODE_LIB_PREFIX}gecode${lib}${GECODE_LIB_SUFFIX}")
  if(lib STREQUAL "float" AND NOT (GECODE_ENABLE_MPFR AND MPFR_FOUND))
    # Keep in sync with Make behavior: skip empty MPFR-only source when MPFR is absent.
    list(REMOVE_ITEM sources "gecode/float/rounding.cpp")
  endif()

  if(NOT sources)
    return()
  endif()

  if(GECODE_BUILD_SHARED)
    add_library(gecode${lib}_shared SHARED ${sources})
    target_compile_definitions(gecode${lib}_shared
      PUBLIC GECODE_NO_AUTOLINK
      PRIVATE GECODE_BUILD_${libupper})
    target_compile_features(gecode${lib}_shared PUBLIC cxx_std_17)
    target_include_directories(gecode${lib}_shared
      PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    if(GECODE_VISIBILITY_COMPILE_OPTION)
      target_compile_options(gecode${lib}_shared PRIVATE ${GECODE_VISIBILITY_COMPILE_OPTION})
    endif()
    set_target_properties(gecode${lib}_shared PROPERTIES
      OUTPUT_NAME "${component_output_name}"
      VERSION ${GECODE_PROJECT_VERSION}
      SOVERSION ${GECODE_SOVERSION})
    if(APPLE)
      set_target_properties(gecode${lib}_shared PROPERTIES
        INSTALL_RPATH "@loader_path")
    elseif(UNIX)
      set_target_properties(gecode${lib}_shared PROPERTIES
        INSTALL_RPATH "$ORIGIN")
    endif()
    add_dependencies(gecode${lib}_shared gecode-varimp-gen)
    list(APPEND GECODE_INSTALL_TARGETS gecode${lib}_shared)
    list(APPEND GECODE_EXPORT_TARGETS gecode${lib}_shared)
    set(GECODE_INSTALL_TARGETS ${GECODE_INSTALL_TARGETS} PARENT_SCOPE)
    set(GECODE_EXPORT_TARGETS ${GECODE_EXPORT_TARGETS} PARENT_SCOPE)
  endif()

  if(GECODE_BUILD_STATIC)
    add_library(gecode${lib}_static STATIC ${sources})
    target_compile_definitions(gecode${lib}_static
      PUBLIC GECODE_NO_AUTOLINK
      PRIVATE GECODE_BUILD_${libupper})
    target_compile_features(gecode${lib}_static PUBLIC cxx_std_17)
    target_include_directories(gecode${lib}_static
      PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    if(GECODE_VISIBILITY_COMPILE_OPTION)
      target_compile_options(gecode${lib}_static PRIVATE ${GECODE_VISIBILITY_COMPILE_OPTION})
    endif()
    set_target_properties(gecode${lib}_static PROPERTIES
      OUTPUT_NAME "${component_output_name}")
    add_dependencies(gecode${lib}_static gecode-varimp-gen)
    list(APPEND GECODE_INSTALL_TARGETS gecode${lib}_static)
    list(APPEND GECODE_EXPORT_TARGETS gecode${lib}_static)
    set(GECODE_INSTALL_TARGETS ${GECODE_INSTALL_TARGETS} PARENT_SCOPE)
    set(GECODE_EXPORT_TARGETS ${GECODE_EXPORT_TARGETS} PARENT_SCOPE)
  endif()

  add_library(gecode${lib} INTERFACE)
  target_link_libraries(gecode${lib} INTERFACE gecode${lib}_${GECODE_DEFAULT_LINK_VARIANT})
  add_library(Gecode::gecode${lib} ALIAS gecode${lib})
  list(APPEND GECODE_INSTALL_TARGETS gecode${lib})
  list(APPEND GECODE_EXPORT_TARGETS gecode${lib})
  set(GECODE_INSTALL_TARGETS ${GECODE_INSTALL_TARGETS} PARENT_SCOPE)
  set(GECODE_EXPORT_TARGETS ${GECODE_EXPORT_TARGETS} PARENT_SCOPE)
endfunction()

function(gecode_link_component comp)
  foreach(kind shared static)
    if(TARGET gecode${comp}_${kind})
      foreach(dep ${ARGN})
        if(TARGET gecode${dep}_${kind})
          target_link_libraries(gecode${comp}_${kind} PUBLIC gecode${dep}_${kind})
        elseif(dep IN_LIST GECODE_LIBRARY_COMPONENTS)
          message(FATAL_ERROR "Component ${comp} requires disabled component ${dep}")
        elseif(TARGET ${dep})
          target_link_libraries(gecode${comp}_${kind} PUBLIC ${dep})
        else()
          target_link_libraries(gecode${comp}_${kind} PUBLIC ${dep})
        endif()
      endforeach()
    endif()
  endforeach()
endfunction()

function(gecode_enable_automoc_if_exists target_name)
  if(TARGET ${target_name})
    set_target_properties(${target_name} PROPERTIES AUTOMOC ON)
  endif()
endfunction()

foreach(component ${GECODE_LIBRARY_COMPONENTS})
  add_gecode_component_library(${component})
endforeach()

if(GECODE_ENABLE_THREAD)
  find_package(Threads REQUIRED)
  gecode_link_component(support Threads::Threads)
endif()

gecode_link_component(kernel support)
if(GECODE_ENABLE_SEARCH)
  gecode_link_component(search kernel)
  if(WIN32 AND GECODE_ENABLE_CPPROFILER)
    gecode_link_component(search ws2_32)
  endif()
endif()
if(GECODE_ENABLE_INT_VARS)
  gecode_link_component(int kernel)
endif()
if(GECODE_ENABLE_SET_VARS)
  gecode_link_component(set int)
endif()
if(GECODE_ENABLE_FLOAT_VARS)
  gecode_link_component(float int kernel)
  if(GECODE_ENABLE_MPFR AND MPFR_FOUND)
    foreach(kind shared static)
      if(TARGET gecodefloat_${kind})
        target_link_libraries(gecodefloat_${kind} PUBLIC MPFR::MPFR)
      endif()
    endforeach()
  endif()
endif()
if(GECODE_ENABLE_MINIMODEL)
  set(mm_deps int set search)
  if(GECODE_ENABLE_FLOAT_VARS)
    list(APPEND mm_deps float)
  endif()
  gecode_link_component(minimodel ${mm_deps})
endif()
if(GECODE_ENABLE_DRIVER)
  set(driver_deps int search)
  if(GECODE_ENABLE_MINIMODEL)
    list(APPEND driver_deps minimodel)
  endif()
  if(GECODE_BUILD_GIST_TARGET)
    list(APPEND driver_deps gist)
  endif()
  gecode_link_component(driver ${driver_deps})
endif()
if(GECODE_BUILD_GIST_TARGET)
  gecode_enable_automoc_if_exists(gecodegist_shared)
  gecode_enable_automoc_if_exists(gecodegist_static)
  foreach(kind shared static)
    if(TARGET gecodegist_${kind})
      target_link_libraries(gecodegist_${kind} PUBLIC ${GECODE_QT_WIDGETS})
    endif()
  endforeach()
  set(gist_deps search int)
  if(GECODE_ENABLE_SET_VARS)
    list(APPEND gist_deps set)
  endif()
  if(GECODE_ENABLE_FLOAT_VARS)
    list(APPEND gist_deps float)
  endif()
  gecode_link_component(gist ${gist_deps})
endif()
if(GECODE_ENABLE_FLATZINC)
  set(fzn_deps minimodel driver)
  if(GECODE_BUILD_GIST_TARGET)
    list(APPEND fzn_deps gist)
  endif()
  gecode_link_component(flatzinc ${fzn_deps})
  if(GECODE_HAS_QT_RUNTIME)
    foreach(kind shared static)
      if(TARGET gecodeflatzinc_${kind})
        target_link_libraries(gecodeflatzinc_${kind} PUBLIC ${GECODE_QT_CORE})
      endif()
    endforeach()
  endif()
endif()

# Compatibility aggregate target for downstream projects expecting Gecode::gecode.
add_library(gecode INTERFACE)
foreach(component IN ITEMS support kernel search int set float minimodel driver flatzinc gist)
  if(TARGET gecode${component})
    target_link_libraries(gecode INTERFACE gecode${component})
  endif()
endforeach()
add_library(Gecode::gecode ALIAS gecode)
list(APPEND GECODE_INSTALL_TARGETS gecode)
list(APPEND GECODE_EXPORT_TARGETS gecode)

# ---------------------------------------------------------------------------
# Executables / tests
# ---------------------------------------------------------------------------
if(GECODE_ENABLE_FLATZINC)
  add_executable(fzn-gecode ${GECODE_FLATZINC_EXE_SOURCE})
  target_link_libraries(fzn-gecode PRIVATE gecodeflatzinc)
  if(APPLE)
    set_target_properties(fzn-gecode PROPERTIES
      INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
  elseif(UNIX)
    set_target_properties(fzn-gecode PROPERTIES
      INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
  endif()
  list(APPEND GECODE_INSTALL_TARGETS fzn-gecode)
endif()

if(BUILD_TESTING)
  set(GECODE_CAN_BUILD_TESTS TRUE)
  foreach(required search int minimodel driver)
    if(NOT TARGET gecode${required})
      set(GECODE_CAN_BUILD_TESTS FALSE)
    endif()
  endforeach()

  if(GECODE_ENABLE_SET_VARS AND NOT TARGET gecodeset)
    set(GECODE_CAN_BUILD_TESTS FALSE)
  endif()
  if(GECODE_ENABLE_FLOAT_VARS AND NOT TARGET gecodefloat)
    set(GECODE_CAN_BUILD_TESTS FALSE)
  endif()
  if(GECODE_ENABLE_FLATZINC AND NOT TARGET gecodeflatzinc)
    set(GECODE_CAN_BUILD_TESTS FALSE)
  endif()

  set(GECODE_TEST_SOURCES_SELECTED ${GECODE_TEST_SOURCES})
  if(NOT GECODE_ENABLE_SET_VARS)
    list(FILTER GECODE_TEST_SOURCES_SELECTED EXCLUDE REGEX "^test/set(/|\\.cpp)")
  endif()
  if(NOT GECODE_ENABLE_FLOAT_VARS)
    list(FILTER GECODE_TEST_SOURCES_SELECTED EXCLUDE REGEX "^test/float(/|\\.cpp)")
  endif()
  if(NOT GECODE_ENABLE_FLATZINC)
    list(FILTER GECODE_TEST_SOURCES_SELECTED EXCLUDE REGEX "^test/flatzinc(/|\\.cpp)")
  endif()

  if(GECODE_CAN_BUILD_TESTS)
    add_executable(gecode-test EXCLUDE_FROM_ALL ${GECODE_TEST_SOURCES_SELECTED})
    set(GECODE_TEST_LINK_LIBS gecodeminimodel)
    if(GECODE_ENABLE_FLATZINC)
      list(APPEND GECODE_TEST_LINK_LIBS gecodeflatzinc)
    endif()
    target_link_libraries(gecode-test PRIVATE ${GECODE_TEST_LINK_LIBS})

    set(GECODE_CHECK_TESTS
      Branch::Int::Dense::3
      Int::Arithmetic::Abs
      Int::Arithmetic::ArgMax
      Int::Arithmetic::Max::Nary
      Int::Cumulative::Man::Fix::0::4
      Int::Distinct::Random
      Int::Linear::Bool::Int::Lq
      Int::MiniModel::LinExpr::Bool::352
      NoGoods::Queens
      Search::DFS::Sol::Binary::Nary::Binary::1::1::1)
    if(GECODE_ENABLE_FLATZINC)
      list(INSERT GECODE_CHECK_TESTS 1 FlatZinc::magic_square)
    endif()
    if(GECODE_ENABLE_SET_VARS)
      list(APPEND GECODE_CHECK_TESTS
        Set::Dom::Dom::Gr
        Set::RelOp::ConstSSI::Union
        Set::Sequence::SeqU1
        Set::Wait)
    endif()

    set(GECODE_CHECK_ARGS -iter 2 -threads 0)
    foreach(gecode_check_test ${GECODE_CHECK_TESTS})
      list(APPEND GECODE_CHECK_ARGS -test ${gecode_check_test})
    endforeach()

    add_test(NAME test COMMAND gecode-test ${GECODE_CHECK_ARGS})
    add_custom_target(check
      COMMAND $<TARGET_FILE:gecode-test> ${GECODE_CHECK_ARGS}
      DEPENDS gecode-test
      USES_TERMINAL)
  else()
    message(WARNING "Skipping gecode-test/check targets because required modules are disabled")
    add_custom_target(check
      COMMAND ${CMAKE_COMMAND} -E echo "Skipping check target because required modules are disabled")
  endif()
else()
  add_custom_target(check
    COMMAND ${CMAKE_COMMAND} -E echo "Skipping check target because BUILD_TESTING is OFF")
endif()

# ---------------------------------------------------------------------------
# FlatZinc scripts and solver configs
# ---------------------------------------------------------------------------
set(prefix ${CMAKE_INSTALL_PREFIX})
set(datarootdir \${prefix}/share)
set(datadir \${datarootdir})
if(GECODE_ENABLE_FLATZINC)
  set(_gecode_install_bindir "${CMAKE_INSTALL_FULL_BINDIR}")
  set(_gecode_install_datadir "${CMAKE_INSTALL_FULL_DATADIR}")
  set(_gecode_install_solverdir "${CMAKE_INSTALL_FULL_DATADIR}/minizinc/solvers")
  set(_gecode_install_mznlibdir "${CMAKE_INSTALL_FULL_DATADIR}/minizinc/gecode")
  set(_gecode_install_fzn_executable "${CMAKE_INSTALL_FULL_BINDIR}/fzn-gecode${CMAKE_EXECUTABLE_SUFFIX}")
  file(RELATIVE_PATH GECODE_MZN_DATADIR_FROM_BINDIR
       "${_gecode_install_bindir}" "${_gecode_install_datadir}")
  file(RELATIVE_PATH GECODE_MSC_MZNLIB_FROM_SOLVERDIR
       "${_gecode_install_solverdir}" "${_gecode_install_mznlibdir}")
  file(RELATIVE_PATH GECODE_MSC_EXECUTABLE_FROM_SOLVERDIR
       "${_gecode_install_solverdir}" "${_gecode_install_fzn_executable}")
  set(GECODE_MSC_STD_FLAGS "\"-a\", \"-f\", \"-n\", \"-p\", \"-r\", \"-s\", \"-t\"")
  if(GECODE_ENABLE_CPPROFILER)
    string(APPEND GECODE_MSC_STD_FLAGS ", \"--cp-profiler\"")
  endif()
  string(REPLACE "/" "\\" GECODE_MZN_DATADIR_FROM_BINDIR_WIN
         "${GECODE_MZN_DATADIR_FROM_BINDIR}")
  if(WIN32)
    configure_file(${PROJECT_SOURCE_DIR}/tools/flatzinc/mzn-gecode.bat.in
                   ${PROJECT_BINARY_DIR}/tools/flatzinc/mzn-gecode.bat
                   @ONLY
                   FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    set(MZN_SCRIPT ${PROJECT_BINARY_DIR}/tools/flatzinc/mzn-gecode.bat)
  else()
    configure_file(${PROJECT_SOURCE_DIR}/tools/flatzinc/mzn-gecode.in
                   ${PROJECT_BINARY_DIR}/tools/flatzinc/mzn-gecode
                   @ONLY
                   FILE_PERMISSIONS
                     OWNER_READ OWNER_WRITE OWNER_EXECUTE
                     GROUP_READ GROUP_EXECUTE
                     WORLD_READ WORLD_EXECUTE)
    set(MZN_SCRIPT ${PROJECT_BINARY_DIR}/tools/flatzinc/mzn-gecode)
  endif()
  configure_file(${PROJECT_SOURCE_DIR}/tools/flatzinc/gecode.msc.in
                 ${PROJECT_BINARY_DIR}/tools/flatzinc/gecode.msc @ONLY)
  if(GECODE_BUILD_GIST_TARGET)
    configure_file(${PROJECT_SOURCE_DIR}/tools/flatzinc/gecode-gist.msc.in
                   ${PROJECT_BINARY_DIR}/tools/flatzinc/gecode-gist.msc @ONLY)
  endif()
endif()

if(GECODE_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# Installation + CMake package export
# ---------------------------------------------------------------------------
if(GECODE_INSTALL)
  include(CMakePackageConfigHelpers)

  install(TARGETS ${GECODE_INSTALL_TARGETS}
    EXPORT GecodeTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  if(GECODE_ENABLE_FLATZINC)
    if(WIN32)
      install(FILES ${MZN_SCRIPT} DESTINATION ${CMAKE_INSTALL_BINDIR})
    else()
      install(PROGRAMS ${MZN_SCRIPT} DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
  endif()

  install(DIRECTORY gecode
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
      PATTERN "**.hh"
      PATTERN "**.hpp"
      PATTERN "LICENSE_1_0.txt"
      PATTERN "mznlib" EXCLUDE
      PATTERN "exampleplugin" EXCLUDE
      PATTERN "standalone-example" EXCLUDE
      PATTERN "abi*" EXCLUDE)

  install(FILES ${PROJECT_BINARY_DIR}/gecode/support/config.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gecode/support/)
  if(GECODE_REGENERATE_VARIMP)
    install(FILES ${GECODE_VAR_TYPE_HPP} ${GECODE_VAR_IMP_HPP}
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gecode/kernel/)
  endif()

  if(GECODE_ENABLE_FLATZINC)
    install(DIRECTORY gecode/flatzinc/mznlib/
      DESTINATION ${CMAKE_INSTALL_DATADIR}/minizinc/gecode)

    install(FILES ${PROJECT_BINARY_DIR}/tools/flatzinc/gecode.msc
      DESTINATION ${CMAKE_INSTALL_DATADIR}/minizinc/solvers)
    if(GECODE_BUILD_GIST_TARGET)
      install(FILES ${PROJECT_BINARY_DIR}/tools/flatzinc/gecode-gist.msc
        DESTINATION ${CMAKE_INSTALL_DATADIR}/minizinc/solvers)
    endif()
  endif()

  set(GECODE_PACKAGE_NEEDS_THREADS OFF)
  if(GECODE_ENABLE_THREAD)
    set(GECODE_PACKAGE_NEEDS_THREADS ON)
  endif()
  set(GECODE_PACKAGE_NEEDS_MPFR OFF)
  if(GECODE_ENABLE_FLOAT_VARS AND GECODE_ENABLE_MPFR AND MPFR_FOUND)
    set(GECODE_PACKAGE_NEEDS_MPFR ON)
  endif()
  set(GECODE_PACKAGE_QT_MAJOR "")
  set(GECODE_PACKAGE_QT_COMPONENTS "")
  set(GECODE_PACKAGE_QT_VERSION "")
  if(GECODE_BUILD_GIST_TARGET)
    set(GECODE_PACKAGE_QT_COMPONENTS "Core;Gui;Widgets;PrintSupport")
  elseif(GECODE_ENABLE_FLATZINC AND GECODE_HAS_QT_RUNTIME)
    set(GECODE_PACKAGE_QT_COMPONENTS "Core")
  endif()
  if(NOT GECODE_PACKAGE_QT_COMPONENTS STREQUAL "")
    set(GECODE_PACKAGE_QT_MAJOR "${GECODE_QT_MAJOR}")
    string(REGEX MATCH "^[0-9]+\\.[0-9]+" GECODE_PACKAGE_QT_VERSION "${GECODE_QT_VERSION}")
  endif()

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GecodeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/GecodeConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Gecode
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR)

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/GecodeConfigVersion.cmake
    VERSION ${GECODE_PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

  install(EXPORT GecodeTargets
    FILE GecodeTargets.cmake
    NAMESPACE Gecode::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Gecode)

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/GecodeConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/GecodeConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Gecode)

  install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/cmake_modules/FindMPFR.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Gecode)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
