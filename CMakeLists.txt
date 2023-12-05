#
#  Main authors:
#     Victor Zverovich <victor.zverovich@gmail.com>
#
#  Copyright:
#     Victor Zverovich, 2013
#
#  This file is part of Gecode, the generic constraint
#  development environment:
#     http://www.gecode.org
#
#  Permission is hereby granted, free of charge, to any person obtaining
#  a copy of this software and associated documentation files (the
#  "Software"), to deal in the Software without restriction, including
#  without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to
#  permit persons to whom the Software is furnished to do so, subject to
#  the following conditions:
#
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
#  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
#  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

#
# CMake build script for Gecode.
#

cmake_minimum_required(VERSION 3.8.0)

project(GECODE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/misc/cmake_modules)

find_package(MPFR)

include(CheckCXXCompilerFlag)
if (GECODE_DISABLE_WARNINGS)
  if (MSVC)
    add_definitions(/wd4244 /wd4267 /wd4345 /wd4355 /wd4800 /wd4068)
  else ()
    foreach (flag -Wextra -Wall -pedantic)
      string(REPLACE ${flag} "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endforeach ()
    if (CMAKE_COMPILER_IS_GNUCXX)
      add_definitions(-Wno-overloaded-virtual -Wno-unknown-pragmas)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      add_definitions(-Wno-constant-logical-operand -Wno-switch -Wno-unknown-pragmas)
    endif ()
  endif ()
endif ()

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.11.0)
  cmake_policy(SET CMP0072 NEW)
  set (OpenGL_GL_PREFERENCE LEGACY)
endif()
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.17.0)
  cmake_policy(SET CMP0100 NEW)
endif()

# The following part of config.h is hard to derive from configure.ac.
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

# Get version numbers and parts of config.h from configure.ac.
file(READ configure.ac LINES)
# Replace semicolons with "<semi>" to avoid CMake messing with them.
string(REPLACE ";" "<semi>" LINES "${LINES}")
# Split into lines keeping newlines to avoid foreach skipping empty ones.
string(REGEX MATCHALL "[^\n]*\n" LINES "${LINES}")
set(ah_command FALSE)
foreach (line "${EXTRA_CONFIG}" ${LINES})
  string(REPLACE ";" "" line "${line}")
  if (ah_command)
    # Do nothing.
  elseif (line MATCHES "AC_INIT\\(([^,]*), *([^,]*), *([^)]*)\\)")
    set(PACKAGE ${CMAKE_MATCH_1})
    set(VERSION ${CMAKE_MATCH_2})
    set(PACKAGE_BUGREPORT ${CMAKE_MATCH_3})
    message(STATUS "Got VERSION=${VERSION} from configure.ac")
  elseif (line MATCHES "ac_gecode_flatzincversion=(.*)\n")
    set(GECODE_FLATZINC_VERSION "${CMAKE_MATCH_1}")
  elseif (line MATCHES "AH_BOTTOM\\(\\[(.*)")
    set(ah_command bottom)
    set(line "${CMAKE_MATCH_1}")
  elseif (line MATCHES "AH_VERBATIM[^,]+,(.*)")
    set(ah_command verbatim)
    set(line "${CMAKE_MATCH_1}")
  endif ()
  if (ah_command)
    set(saved_ah_command ${ah_command})
    if (line MATCHES "^\\[(.*)")
      set(line "${CMAKE_MATCH_1}")
    endif ()
    if (line MATCHES "\\]\\)")
      set(ah_command FALSE)
      string(REPLACE "])" "" line "${line}")
    endif ()
    # For some reason CMake may bundle several lines together. Split them too.
    string(REGEX MATCHALL "[^\n]*\n" sublines "${line}")
    set(config_add "")
    foreach (subline ${sublines})
      set(config_add ${config_add} "${subline}")
    endforeach ()
    if (saved_ah_command STREQUAL "verbatim")
      set(CONFIG ${config_add} ${CONFIG})
    else ()
      set(CONFIG ${CONFIG} "\n" ${config_add})
    endif ()
  endif ()
endforeach ()
set(PACKAGE_NAME ${PACKAGE})
string(TOLOWER ${PACKAGE} PACKAGE_TARNAME)
set(PACKAGE_URL "")
set(PACKAGE_VERSION ${VERSION})
set(${PACKAGE}_VERSION ${VERSION})
string(REPLACE "." "-" GECODE_LIBRARY_VERSION "${VERSION}")
set(PACKAGE_STRING "${PACKAGE} ${VERSION}")
if (VERSION MATCHES "(.*)\\.(.*)\\.(.*)")
  math(EXPR GECODE_VERSION_NUMBER
    "${CMAKE_MATCH_1} * 100000 + ${CMAKE_MATCH_2} * 100 + ${CMAKE_MATCH_3}")
endif ()

set(GECODE_DLL_USERPREFIX "")
set(GECODE_DLL_USERSUFFIX "")
set(GECODE_HAS_INT_VARS "/**/")
set(GECODE_HAS_SET_VARS "/**/")
set(GECODE_HAS_FLOAT_VARS "/**/")
set(GECODE_STATIC_LIBS 1)
set(GECODE_ALLOCATOR "/**/")
if (MPFR_FOUND)
  set(GECODE_HAS_MPFR "/**/")
endif ()

check_cxx_compiler_flag(-fvisibility=hidden HAVE_VISIBILITY_HIDDEN_FLAG)
if (HAVE_VISIBILITY_HIDDEN_FLAG)
  set(GECODE_GCC_HAS_CLASS_VISIBILITY "/**/")
endif ()

option(ENABLE_THREADS "Enable thread support" ON)
if (ENABLE_THREADS)
  set(GECODE_HAS_THREADS 1)
endif()

option(ENABLE_GIST "Enable gist" OFF)
if (ENABLE_GIST)
  set(GECODE_USE_QT TRUE)
else()
  set(GECODE_USE_QT FALSE)
endif()

# Don't use Qt if GECODE_USE_QT is set to FALSE.
if (NOT DEFINED GECODE_USE_QT OR GECODE_USE_QT)
  find_package(Qt6 QUIET COMPONENTS Core Gui Widgets PrintSupport)
  if (Qt6_FOUND)
	  set(GECODE_HAS_QT "/**/")
	  set(GECODE_HAS_GIST "/**/")
	  set(EXTRA_LIBS gist)
	  set(CMAKE_AUTOMOC TRUE)
	else()
		find_package(Qt5 QUIET COMPONENTS Core Gui Widgets PrintSupport)
		if (Qt5_FOUND)
			 set(GECODE_HAS_QT "/**/")
			 set(GECODE_HAS_GIST "/**/")
			 set(EXTRA_LIBS gist)
			 set(CMAKE_AUTOMOC TRUE)
		else()
			find_package(Qt4)
			if (QT4_FOUND)
				set(GECODE_HAS_QT "/**/")
				set(GECODE_HAS_GIST "/**/")
				set(EXTRA_LIBS gist)
				set(CMAKE_AUTOMOC TRUE)
				include(${QT_USE_FILE})
			endif()
		endif()
  endif()
endif ()

include(CheckSymbolExists)
check_symbol_exists(mmap sys/mman.h HAVE_MMAP)

# Check for inline.
include(CheckCSourceCompiles)
check_c_source_compiles("
  inline __attribute__ ((__always_inline__)) void foo(void) {}
  int main() {}" HAVE_ALWAYS_INLINE)
check_c_source_compiles("
  __forceinline void foo(void) {}
  int main() {}" HAVE_FORCE_INLINE)
set(forceinline inline)
if (HAVE_ALWAYS_INLINE)
  set(forceinline "inline __attribute__ ((__always_inline__))")
endif ()
if (HAVE_FORCE_INLINE)
  set(forceinline "__forceinline")
endif ()

# Check for bit index
check_c_source_compiles("
  int main() { return __builtin_ffsl(0); }" HAVE_BUILTIN_FFSL)
if (HAVE_BUILTIN_FFSL)
  set(GECODE_HAS_BUILTIN_FFSL "/**/")
endif ()

# Check for popcount
check_c_source_compiles("
  int main() { return __builtin_popcountll(0); }" HAVE_BUILTIN_POPCOUNTLL)
if (HAVE_BUILTIN_POPCOUNTLL)
  set(GECODE_HAS_BUILTIN_POPCOUNTLL "/**/")
endif ()

# Process config.hpp using autoconf rules.
list(LENGTH CONFIG length)
math(EXPR length "${length} - 1")
foreach (i RANGE ${length})
  list(GET CONFIG ${i} line)
  if (line MATCHES "^#( *)undef (.*)\n")
    set(space "${CMAKE_MATCH_1}")
    set(var ${CMAKE_MATCH_2})
    if (NOT DEFINED ${var} OR (var MATCHES "HAVE_.*" AND NOT ${var}))
      set(line "/* #${space}undef ${var} */\n")
    else ()
      if ("${${var}}" STREQUAL "/**/" OR "${var}" STREQUAL "GECODE_VERSION_NUMBER" OR
          "${var}" STREQUAL "forceinline" OR var MATCHES "SIZEOF_.*")
        set(value ${${var}})
      elseif (NOT (var MATCHES ^HAVE OR ${var} EQUAL 0 OR ${var} EQUAL 1))
        set(value \"${${var}}\")
      elseif (${var})
        set(value 1)
      else ()
        set(value 0)
      endif ()
      set(line "#${space}define ${var} ${value}\n")
    endif ()
  endif ()
  string(REPLACE "<semi>" ";" line "${line}")
  set(CONFIG_OUT "${CONFIG_OUT}${line}")
endforeach ()
file(WRITE ${GECODE_BINARY_DIR}/gecode/support/config.hpp
"/* gecode/support/config.hpp.  Generated from config.hpp.in by configure.  */
/* gecode/support/config.hpp.in.  Generated from configure.ac by autoheader.  */

/* Disable autolink because all the dependencies are handled by CMake. */
#define GECODE_BUILD_SUPPORT
#define GECODE_BUILD_KERNEL
#define GECODE_BUILD_SEARCH
#define GECODE_BUILD_INT
#define GECODE_BUILD_SET
#define GECODE_BUILD_FLOAT
#define GECODE_BUILD_MINIMODEL
#define GECODE_BUILD_FLATZINC
#define GECODE_BUILD_DRIVER
#define GECODE_BUILD_GIST

${CONFIG_OUT}")

# Expands a value substituting variables and appends the result to ${var}.
function (expand var value)
  if (value MATCHES "\\$\\(([^:]+)(.*)\\)")
    # Perform substitution.
    set(pattern ${CMAKE_MATCH_2})
    set(items ${${CMAKE_MATCH_1}})
    if (pattern MATCHES ":%=([^%]*)%([^%]*)")
      set(values )
      foreach (item ${items})
        set(values ${values} ${CMAKE_MATCH_1}${item}${CMAKE_MATCH_2})
      endforeach ()
    else ()
      set(values ${items})
    endif ()
  else ()
    set(values ${value})
  endif ()
  set(${var} ${${var}} ${values} PARENT_SCOPE)
endfunction ()

# Parse Makefile.in extracting variables.
file(READ Makefile.in text)
string(REPLACE "\\\n" "" text "${text}")
string(REGEX REPLACE "#[^\n]*\n" "" text "${text}")
string(REGEX MATCHALL "[^\n]+" lines "${text}")
foreach (line ${lines})
  if (line MATCHES "([^ \t]+)[ \t]*=[ \t]*(.*)")
    set(var ${CMAKE_MATCH_1})
    set(${var} )
    string(REGEX MATCHALL "[^ \t]+" items "${CMAKE_MATCH_2}")
    foreach (item ${items})
      expand(${var} ${item})
    endforeach ()
  endif ()
endforeach ()

foreach (lib support kernel search int set float
             minimodel driver flatzinc ${EXTRA_LIBS})
  if (lib STREQUAL "minimodel")
    set(libupper MM)
  else ()
    string(TOUPPER ${lib} libupper)
  endif ()
  if (${libupper}SRC)
    set(sources )
    foreach (src ${${libupper}SRC} ${${libupper}_GENSRC})
      if (src STREQUAL "gecode/float/rounding.cpp" AND NOT ${MPFR_FOUND})
        # ignore empty source files to prevent linker warnings
      else ()
        set(sources ${sources} ${src})
      endif ()
    endforeach ()
    add_library(gecode${lib} ${sources} ${${libupper}HDR})
    target_include_directories(gecode${lib}
      PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
    list(APPEND GECODE_INSTALL_TARGETS gecode${lib})
  endif ()
endforeach ()

option(ENABLE_CPPROFILER "Enable cpprofiler tracer mode" ON)
if(ENABLE_CPPROFILER)
  add_definitions(-DGECODE_HAS_CPPROFILER)
endif()

find_package(Threads)
target_link_libraries(gecodesupport ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(gecodekernel gecodesupport)
target_link_libraries(gecodesearch gecodekernel)
target_link_libraries(gecodeint gecodekernel)
target_link_libraries(gecodeset gecodeint)
target_link_libraries(gecodefloat gecodeint)
target_link_libraries(gecodeminimodel gecodeint gecodeset gecodesearch)
target_link_libraries(gecodedriver gecodeint)
target_link_libraries(gecodeflatzinc gecodeminimodel gecodedriver)

if (GECODE_HAS_QT)
  if (Qt6_FOUND)
		target_link_libraries(gecodegist Qt6::Widgets Qt6::Gui Qt6::PrintSupport)
		target_link_libraries(gecodeflatzinc Qt6::Core)
    target_link_libraries(gecodeflatzinc gecodegist)
	elseif (Qt5_FOUND)
		target_link_libraries(gecodegist Qt5::Widgets Qt5::Gui Qt5::PrintSupport)
		target_link_libraries(gecodeflatzinc Qt5::Core)
    target_link_libraries(gecodeflatzinc gecodegist)
  else()
    target_link_libraries(gecodegist ${QT_LIBRARIES})
    target_link_libraries(gecodeflatzinc gecodegist ${QT_LIBRARIES})
  endif()
endif ()

if (FLOATSRC)
  target_link_libraries(gecodefloat gecodekernel)
  target_link_libraries(gecodeminimodel gecodefloat)
  if (MPFR_FOUND)
    target_link_libraries(gecodefloat ${MPFR_LIBRARIES})
    target_include_directories(gecodefloat PRIVATE ${MPFR_INCLUDES})
  endif ()
endif ()

add_executable(gecode-test EXCLUDE_FROM_ALL ${TESTSRC} ${TESTHDR})
target_link_libraries(gecode-test gecodeflatzinc gecodeminimodel)

add_executable(fzn-gecode ${FLATZINCEXESRC})
target_link_libraries(fzn-gecode gecodeflatzinc gecodeminimodel gecodedriver)
list(APPEND GECODE_INSTALL_TARGETS fzn-gecode)

set(prefix ${CMAKE_INSTALL_PREFIX})
set(datarootdir \${prefix}/share)
set(datadir \${datarootdir})
if(WIN32)
  configure_file(
    ${PROJECT_SOURCE_DIR}/tools/flatzinc/mzn-gecode.bat.in
    ${PROJECT_BINARY_DIR}/tools/flatzinc/mzn-gecode.bat
    @ONLY
  )
  set(MZN_SCRIPT ${PROJECT_BINARY_DIR}/tools/flatzinc/mzn-gecode.bat)
else()
  configure_file(
    ${PROJECT_SOURCE_DIR}/tools/flatzinc/mzn-gecode.in
    ${PROJECT_BINARY_DIR}/tools/flatzinc/mzn-gecode
    @ONLY
  )
  set(MZN_SCRIPT ${PROJECT_BINARY_DIR}/tools/flatzinc/mzn-gecode)
endif()
configure_file(
  ${PROJECT_SOURCE_DIR}/tools/flatzinc/gecode.msc.in
  ${PROJECT_BINARY_DIR}/tools/flatzinc/gecode.msc
  @ONLY
)
if (GECODE_HAS_GIST)
	configure_file(
		${PROJECT_SOURCE_DIR}/tools/flatzinc/gecode-gist.msc.in
		${PROJECT_BINARY_DIR}/tools/flatzinc/gecode-gist.msc
		@ONLY
	)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_EXAMPLES "Build examples." OFF) 
if (${BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()

enable_testing()
add_test(test gecode-test
  -iter 2 -test Branch::Int::Dense::3
  -test Int::Linear::Int::Int::Eq::Bnd::12::4
  -test Int::Distinct::Random
  -test Int::Arithmetic::Mult::XYZ::Bnd::C
  -test Int::Arithmetic::Mult::XYZ::Dom::A
  -test Search::BAB::Sol::BalGr::Binary::Binary::Binary::1::1)

## Installation Target
include(GNUInstallDirs)
# Install libraries and executables
install(
  TARGETS ${GECODE_INSTALL_TARGETS}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
  FILES ${MZN_SCRIPT}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# Install include directory
install(
  DIRECTORY gecode
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING
    PATTERN "**.hh"
    PATTERN "**.hpp"
    PATTERN "LICENSE_1_0.txt"
    PATTERN "mznlib" EXCLUDE
    PATTERN "exampleplugin" EXCLUDE
    PATTERN "standalone-example" EXCLUDE
    PATTERN "abi*" EXCLUDE
)
install(
  FILES ${PROJECT_BINARY_DIR}/gecode/support/config.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gecode/support/
)
# Install MiniZinc library
install(
  DIRECTORY gecode/flatzinc/mznlib/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/minizinc/gecode
)
install(
  FILES ${PROJECT_BINARY_DIR}/tools/flatzinc/gecode.msc
  DESTINATION ${CMAKE_INSTALL_DATADIR}/minizinc/solvers
)
if (GECODE_HAS_GIST)
	install(
		FILES ${PROJECT_BINARY_DIR}/tools/flatzinc/gecode-gist.msc
		DESTINATION share/minizinc/solvers
	)
endif()
