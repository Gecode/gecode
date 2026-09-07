# Pure post-build/install JSON configuration, after target-file expansion.
# cmake -DTEMPLATE=... -DVERSION=... -DDRIVER=... -DMZNLIB=... -DOUTPUT=... -P this-file
cmake_minimum_required(VERSION 3.16)

foreach(required TEMPLATE VERSION DRIVER MZNLIB OUTPUT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "Required input ${required} is missing or empty")
  endif()
endforeach()
if(NOT EXISTS "${TEMPLATE}" OR IS_DIRECTORY "${TEMPLATE}")
  message(FATAL_ERROR "TEMPLATE is not an existing file")
endif()

# JSON strings require escaping all U+0001..U+001F controls, not just newlines.
# NUL cannot be supplied in an OS command-line argument/CMake string. UTF-8
# characters otherwise remain intact. Do not normalize relative path spelling.
function(json_string input output)
  string(REPLACE "\\" "\\\\" encoded "${input}")
  string(REPLACE "\"" "\\\"" encoded "${encoded}")
  set(hex "0123456789abcdef")
  foreach(code RANGE 1 31)
    string(ASCII ${code} control)
    math(EXPR high "${code} / 16")
    math(EXPR low "${code} % 16")
    string(SUBSTRING "${hex}" ${high} 1 high_hex)
    string(SUBSTRING "${hex}" ${low} 1 low_hex)
    string(REPLACE "${control}" "\\u00${high_hex}${low_hex}" encoded "${encoded}")
  endforeach()
  set(${output} "${encoded}" PARENT_SCOPE)
endfunction()

json_string("${VERSION}" GECODE_VERSION)
json_string("${DRIVER}" GECODE_OPTIMIZE_MSC_EXECUTABLE)
json_string("${MZNLIB}" GECODE_OPTIMIZE_MSC_MZNLIB)
get_filename_component(output_directory "${OUTPUT}" DIRECTORY)
if(NOT "${output_directory}" STREQUAL "")
  file(MAKE_DIRECTORY "${output_directory}")
endif()
# configure_file substitutes the original template once; placeholder-like text
# inside values stays literal, unlike a sequence of string(REPLACE) operations.
string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef nonce)
set(temporary "${OUTPUT}.${nonce}.tmp")
configure_file("${TEMPLATE}" "${temporary}" @ONLY)
file(RENAME "${temporary}" "${OUTPUT}")
