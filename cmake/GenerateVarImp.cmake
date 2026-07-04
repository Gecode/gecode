if(NOT DEFINED UV_EXECUTABLE OR NOT DEFINED GENVARIMP OR NOT DEFINED MODE OR NOT DEFINED OUT_FILE)
  message(FATAL_ERROR "GenerateVarImp.cmake requires UV_EXECUTABLE, GENVARIMP, MODE, and OUT_FILE")
endif()

if(DEFINED VIS_FILES_SERIALIZED)
  string(REPLACE "||" ";" VIS_FILES "${VIS_FILES_SERIALIZED}")
endif()

execute_process(
  COMMAND "${UV_EXECUTABLE}" run --script "${GENVARIMP}" "${MODE}" ${VIS_FILES}
  OUTPUT_FILE "${OUT_FILE}"
  RESULT_VARIABLE genvarimp_status)

if(NOT genvarimp_status EQUAL 0)
  message(FATAL_ERROR "genvarimp failed for ${OUT_FILE}")
endif()
