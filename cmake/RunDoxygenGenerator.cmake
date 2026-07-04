if(NOT DEFINED UV_EXECUTABLE)
  message(FATAL_ERROR "UV_EXECUTABLE is required")
endif()
if(NOT DEFINED SCRIPT)
  message(FATAL_ERROR "SCRIPT is required")
endif()
if(NOT DEFINED OUT_FILE)
  message(FATAL_ERROR "OUT_FILE is required")
endif()

set(command "${UV_EXECUTABLE}" run --script "${SCRIPT}")

if(DEFINED INPUT_LIST)
  list(APPEND command --input-list "${INPUT_LIST}")
endif()

if(DEFINED INPUT_FILE)
  execute_process(
    COMMAND ${command}
    INPUT_FILE "${INPUT_FILE}"
    OUTPUT_FILE "${OUT_FILE}"
    RESULT_VARIABLE generator_status)
else()
  execute_process(
    COMMAND ${command}
    OUTPUT_FILE "${OUT_FILE}"
    RESULT_VARIABLE generator_status)
endif()

if(NOT generator_status EQUAL 0)
  message(FATAL_ERROR "Doxygen helper generation failed for ${OUT_FILE}")
endif()
