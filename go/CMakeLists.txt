CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
project(go)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

find_package(pybind11 CONFIG REQUIRED)

# add elf and vendor
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../elf/ ${CMAKE_BINARY_DIR}/elf/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/ ${CMAKE_BINARY_DIR}/vendor/)

# the python lib
file(GLOB SOURCES *.cc)
pybind11_add_module(go_game ${SOURCES})
target_link_libraries(go_game PRIVATE elf)
set_target_properties(go_game
	PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
