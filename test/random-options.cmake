#
#  Main authors:
#     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
#
#  Copyright:
#     Mikael Zayenz Lagerkvist, 2026
#
#  This file is part of Gecode, the generic constraint
#  development environment:
#     http://www.gecode.dev
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

set(modes driver)
if(FLATZINC)
  list(APPEND modes flatzinc)
endif()
foreach(mode IN LISTS modes)
  set(prefix)
  set(seed -seed)
  if(mode STREQUAL flatzinc)
    set(prefix flatzinc)
    set(seed -r)
  endif()
  execute_process(COMMAND "${OPTIONS}" ${prefix} ${seed} 18446744073709551615
    RESULT_VARIABLE result OUTPUT_VARIABLE expected ERROR_VARIABLE error)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "64-bit seed failed: ${error}")
  endif()
  string(REGEX MATCH "^[^\n]+" state "${expected}")
  if(NOT state MATCHES "-v1:ffffffffffffffff(:|$)")
    message(FATAL_ERROR "Seed was narrowed: ${state}")
  endif()
  foreach(args "${seed};0xffffffffffffffff" "-state;${state}")
    execute_process(COMMAND "${OPTIONS}" ${prefix} ${args}
      RESULT_VARIABLE result OUTPUT_VARIABLE actual ERROR_VARIABLE error)
    if(NOT result EQUAL 0 OR NOT actual STREQUAL expected)
      message(FATAL_ERROR "State/hex replay differs: ${actual} ${error}")
    endif()
  endforeach()
  if(state MATCHES "^splitmix")
    set(arbitrary "splitmix-v1:fedcba9876543210:0123456789abcdef")
    set(incompatible "xorshift64star-v1:0000000000000001")
  else()
    set(arbitrary "xorshift64star-v1:fedcba9876543210")
    set(incompatible "splitmix-v1:0000000000000000:9e3779b97f4a7c15")
  endif()
  execute_process(COMMAND "${OPTIONS}" ${prefix} -state "${arbitrary}"
    RESULT_VARIABLE result OUTPUT_VARIABLE actual ERROR_VARIABLE error)
  string(REGEX MATCH "^[^\n]+" restored "${actual}")
  if(NOT result EQUAL 0 OR NOT restored STREQUAL arbitrary)
    message(FATAL_ERROR "Complete state was not restored: ${actual} ${error}")
  endif()
  foreach(args "${seed};-1" "${seed};18446744073709551616"
      "${seed};3x" "-state;bad" "-state"
      "-state;${incompatible}"
      "${seed};1;-state;${state}" "-state;${state};${seed};1")
    execute_process(COMMAND "${OPTIONS}" ${prefix} ${args}
      RESULT_VARIABLE result OUTPUT_VARIABLE actual ERROR_VARIABLE error)
    if(result EQUAL 0)
      message(FATAL_ERROR "Invalid options accepted: ${args}")
    endif()
  endforeach()
  foreach(source time hw)
    execute_process(COMMAND "${OPTIONS}" ${prefix} ${seed} ${source}
      RESULT_VARIABLE result OUTPUT_VARIABLE expected ERROR_VARIABLE report)
    string(REGEX MATCH "Random state: -state ([^\n]+)" matched "${report}")
    if(NOT result EQUAL 0 OR NOT matched)
      message(FATAL_ERROR "${source} initialization did not report state: ${report}")
    endif()
    execute_process(COMMAND "${OPTIONS}" ${prefix} -state "${CMAKE_MATCH_1}"
      RESULT_VARIABLE result OUTPUT_VARIABLE actual ERROR_VARIABLE error)
    if(NOT result EQUAL 0 OR NOT actual STREQUAL expected)
      message(FATAL_ERROR "${source} state replay differs: ${actual} ${error}")
    endif()
  endforeach()
endforeach()
