#
# Main authors:
#    Vincent Barichard <Vincent.Barichard@univ-angers.fr>
#
# Copyright:
#    Vincent Barichard, 2013
#
# Last modified:
#    $Date$ by $Author$
#    $Revision$
#
# This file is part of Quacode:
#    http://quacode.barichard.com
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(GECODE_SRC "${CMAKE_SOURCE_DIR}/../.." CACHE PATH "Path where GeCode source is installed")
SET(GECODE_BIN "${GECODE_SRC}" CACHE PATH "Path where GeCode libs and binaries are installed")
SET(BUILD_EXAMPLES ON CACHE BOOL "Build examples or not")

# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take their definition
# and dump it in the cache along with proper documentation, otherwise set CMAKE_BUILD_TYPE
# to Debug prior to calling PROJECT()
#
IF(NOT DEFINED CMAKE_BUILD_TYPE)
  # Check if Gecode is configured with --enable-debug
  FILE(STRINGS ${GECODE_BIN}/config.status GECODE_DEBUG_BUILD REGEX "S\\[\"DEBUG_BUILD\"\\]=")
  IF(GECODE_DEBUG_BUILD MATCHES "yes")
    SET(QUACODE_BUILD_TYPE "Debug")
  ELSE()
    SET(QUACODE_BUILD_TYPE "Release")
  ENDIF()

   SET(CMAKE_BUILD_TYPE "${QUACODE_BUILD_TYPE}" CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ENDIF()

PROJECT(Quacode)
SET(CMAKE_CXX_FLAGS "-std=c++11")

SET(CMAKE_VERBOSE_MAKEFILE TRUE)

# Check if Gecode is configured with --enable-audit
FILE(STRINGS ${GECODE_BIN}/config.status GECODE_AUDIT REGEX "D\\[\"GECODE_AUDIT\"\\]=")
IF(GECODE_AUDIT)
  SET(QUACODE_AUDIT TRUE)
ELSE()
  SET(QUACODE_AUDIT FALSE)
ENDIF()
SET(LOG_AUDIT ${QUACODE_AUDIT} CACHE BOOL "Set to true to generate log output.")

IF(UNIX)
  # determine, whether we want a static binary
  SET(STATIC_LINKING FALSE CACHE BOOL "Build a static binary?")
  # do we want static libraries?
  IF(STATIC_LINKING)
    SET(BUILD_SHARED_LIBS OFF)
    # To know in source file that we compil static
    ADD_DEFINITIONS(-DQUACODE_STATIC_LIBS)
    # When STATIC_LINKING is TRUE, than cmake looks for libraries ending
    # with .a. This is for linux only!
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
    # Remove flags to get rid off all the -Wl,Bydnamic
    SET(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
    SET(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
    # Use static libs for Boost
    SET(Boost_USE_STATIC_LIBS ON)
    SET(Boost_USE_STATIC_RUNTIME ON)
  ELSE(STATIC_LINKING)
    SET(BUILD_SHARED_LIBS ON)
  ENDIF(STATIC_LINKING)
ELSE(UNIX)
  SET(BUILD_SHARED_LIBS ON)
ENDIF(UNIX)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})
FIND_PACKAGE(Gecode)

IF(NOT GECODE_FOUND)
  MESSAGE(FATAL_ERROR "Gecode is needed, consider to install it")
ELSE (NOT GECODE_FOUND)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
  INCLUDE_DIRECTORIES(${GECODE_BIN})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
  INCLUDE_DIRECTORIES(${GECODE_SRC})

  LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
  LINK_DIRECTORIES(${GECODE_BIN})

  IF (CMAKE_COMPILER_IS_GNUCXX)
    ADD_DEFINITIONS(-Wall)
    ADD_DEFINITIONS(-Wextra)
    ADD_DEFINITIONS(-Wno-unused-local-typedefs)
    ADD_DEFINITIONS(-fimplement-inlines)
    ADD_DEFINITIONS(-fno-inline-functions)
    ADD_DEFINITIONS(-pipe)
    ADD_DEFINITIONS(-fPIC)
    SET(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
  ENDIF ()

  IF (LOG_AUDIT)
    ADD_DEFINITIONS(-DLOG_AUDIT)
  ENDIF()

  INCLUDE(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fvisibility=hidden HAVE_VISIBILITY_HIDDEN_FLAG)
  IF (HAVE_VISIBILITY_HIDDEN_FLAG)
    ADD_DEFINITIONS(-fvisibility=hidden)
    ADD_DEFINITIONS(-DQUACODE_GCC_HAS_CLASS_VISIBILITY)
  ENDIF()

  FIND_PACKAGE(Threads)

  SET(QUACODE_HEADERS
    quacode/qcsp.hh
    quacode/qspaceinfo.hh
    quacode/support/dynamic-list.hh
    quacode/support/log.hh
    quacode/search/sequential/qpath.hh
    quacode/search/sequential/qdfs.hh
    quacode/qint/qbool.hh
  )
  SET(QUACODE_HPP
    quacode/qspaceinfo.hpp
    quacode/search/qdfs.hpp
    quacode/qint/watch.hpp
    quacode/qint/qbool/clause.hpp
    quacode/qint/qbool/eq.hpp
    quacode/qint/qbool/eqv.hpp
    quacode/qint/qbool/or.hpp
    quacode/qint/qbool/xor.hpp
    quacode/qint/qbool/xorv.hpp
  )
  SET(QUACODE_SRCS
    quacode/qspaceinfo.cpp
    quacode/support/log.cpp
    quacode/search/qdfs.cpp
    quacode/search/sequential/qpath.cpp
    quacode/qint/qbool/qbool.cpp
    ${GECODE_SRC}/gecode/search/meta/nogoods.cpp
  )
  SET(QUACODE_EXAMPLES_SRCS
    examples/qbf.cpp
    examples/qdimacs.cpp
    examples/nim-fibo.cpp
    examples/matrix-game.cpp
    examples/connect-four.cpp
    examples/baker.cpp
    examples/rndQCSP.cpp
  )

  SOURCE_GROUP("Hpp Files" REGULAR_EXPRESSION ".hpp")

  SET_SOURCE_FILES_PROPERTIES(${ALL_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
  SET_SOURCE_FILES_PROPERTIES(${ALL_HPP} PROPERTIES HEADER_FILE_ONLY TRUE)

  ADD_LIBRARY(quacode ${QUACODE_SRCS} ${QUACODE_HEADERS} ${QUACODE_HPP})
  TARGET_LINK_LIBRARIES(quacode ${GECODE_LIBRARIES})
  SET_TARGET_PROPERTIES(quacode PROPERTIES COMPILE_DEFINITIONS "BUILD_QUACODE_LIB")
  INSTALL(TARGETS quacode LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static)
  SET(QUACODE_LIBRARIES quacode)

  IF(BUILD_EXAMPLES)
    # Add targets for examples
    FOREACH (example ${QUACODE_EXAMPLES_SRCS})
      GET_FILENAME_COMPONENT(exampleBin ${example} NAME_WE)
      ADD_EXECUTABLE(${exampleBin} ${example})
      TARGET_LINK_LIBRARIES(${exampleBin} ${QUACODE_LIBRARIES} ${GECODE_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
      INSTALL(TARGETS ${exampleBin} RUNTIME DESTINATION bin)

      # set -static, when STATIC_LINKING is TRUE and set LINK_SEARCH_END_STATIC
      # to remove the additional -bdynamic from the linker line.
      IF(UNIX AND STATIC_LINKING)
          SET(CMAKE_EXE_LINKER_FLAGS "-static")
          SET_TARGET_PROPERTIES(${exampleBin} PROPERTIES LINK_SEARCH_END_STATIC 1)
      ENDIF(UNIX AND STATIC_LINKING)
    ENDFOREACH ()
  ENDIF(BUILD_EXAMPLES)

ENDIF(NOT GECODE_FOUND)
