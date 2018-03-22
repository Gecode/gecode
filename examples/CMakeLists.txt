#
#  Main authors:
#     Petter Strandmark <petter.strandmark@gmail.com>
#
#  Copyright:
#     Petter Strandmark, 2014
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
# CMake build script for Gecode examples.
#

if (MSVC)
	# Needed for some examples to prevent
	# C1128: number of sections exceeded object file format limit : compile with /bigobj
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

macro (GECODE_EXAMPLE NAME)
	add_executable(${NAME}${EXECUTABLE_EXTENSION}
	               ${NAME}.cpp)
	target_link_libraries(${NAME}${EXECUTABLE_EXTENSION} gecodeminimodel gecodedriver)
	set_property(TARGET ${NAME}${EXECUTABLE_EXTENSION}         PROPERTY FOLDER "Examples")
endmacro ()

file(GLOB EXAMPLE_FILES *.cpp)
foreach (EXAMPLE_FILE ${EXAMPLE_FILES})
	get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
	message("-- Adding example: " ${EXAMPLE_NAME})
	gecode_example(${EXAMPLE_NAME})
endforeach()
