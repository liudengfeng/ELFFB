# find sources
file(GLOB SOURCES lib/*.cc tar_loader.cc)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# require at least gcc 4.9
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
		message(FATAL_ERROR "GCC version must be at least 4.9!")
	endif()
endif()

add_library(elf INTERFACE)
target_sources(elf INTERFACE ${SOURCES})

find_package(TBB CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)

target_link_libraries(elf INTERFACE 
	unofficial::concurrentqueue::concurrentqueue 
	TBB::tbb 
	microtar
)
target_include_directories(elf
	INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
)

set(USE_TBB ON)
message(STATUS "使用TBB：${USE_TBB}")

target_compile_definitions(elf INTERFACE USE_TBB)


