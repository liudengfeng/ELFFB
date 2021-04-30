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
target_link_libraries(elf INTERFACE concurrentqueue tbb microtar)
target_include_directories(elf
	INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
	)

target_compile_definitions(elf INTERFACE USE_TBB)


