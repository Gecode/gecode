cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(gecode_gist_example LANGUAGES CXX)
  find_package(Gecode CONFIG REQUIRED COMPONENTS gist)
endif()

add_executable(mygist mygist.cpp)
target_link_libraries(mygist PRIVATE Gecode::gecodegist)
