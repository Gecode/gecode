if(NOT DEFINED TEST_EXECUTABLE OR NOT DEFINED REQUIRED_TESTS)
  message(FATAL_ERROR "TEST_EXECUTABLE and REQUIRED_TESTS are required")
endif()

execute_process(
  COMMAND "${TEST_EXECUTABLE}" -list
  RESULT_VARIABLE list_result
  OUTPUT_VARIABLE registered_tests
  ERROR_VARIABLE list_error)
if(NOT list_result EQUAL 0)
  message(FATAL_ERROR
    "Could not list registered Gecode tests (${list_result}): ${list_error}")
endif()

string(REPLACE "\r\n" "\n" registered_tests "${registered_tests}")
string(REPLACE "\r" "\n" registered_tests "${registered_tests}")
set(registered_tests "\n${registered_tests}\n")
foreach(required_test IN LISTS REQUIRED_TESTS)
  string(FIND "${registered_tests}" "\n${required_test}\n" test_position)
  if(test_position EQUAL -1)
    message(FATAL_ERROR
      "Required check test is not registered: ${required_test}")
  endif()
endforeach()
