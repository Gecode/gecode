# Developer checks are explicit targets; ordinary builds need no LLVM tools.
if(GECODE_UV_EXECUTABLE)
  set(_gecode_check_uv "${GECODE_UV_EXECUTABLE}")
else()
  set(_gecode_check_uv uv)
endif()

foreach(command IN ITEMS format format-check lint all)
  if(command STREQUAL "format")
    set(target format)
  elseif(command STREQUAL "format-check")
    set(target check-format)
  elseif(command STREQUAL "lint")
    set(target check-lint)
  else()
    set(target check-extended)
  endif()
  add_custom_target(${target}
    COMMAND "${_gecode_check_uv}" run "${PROJECT_SOURCE_DIR}/misc/check.py" ${command}
            --build-dir "${PROJECT_BINARY_DIR}" --config "$<CONFIG>"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    USES_TERMINAL
    VERBATIM)
endforeach()
unset(_gecode_check_uv)
