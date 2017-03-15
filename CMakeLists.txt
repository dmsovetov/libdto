# Set the minimum CMake version
cmake_minimum_required(VERSION 3.2)

# Declare the project
project(libdto)

# Declare options
option(DTO_TESTS "Build DTO unit tests" OFF)

# Library source files
set(SRC
	Dto.cpp
	Bson.cpp
	Json.cpp
	Yaml.cpp
	ByteBuffer.cpp
	)
	
# Library header files
set(HEADERS
	Dto.h
	Bson.h
	Json.h
	Yaml.h
	ByteBuffer.h
	)
	
# Configure IDE source file filters
source_group("Code" FILES ${SRC} ${HEADERS})
	
# Add libdto static library
add_library(libdto STATIC ${SRC} ${HEADERS})

# Add a test executable
if (DTO_TESTS)
	add_subdirectory(tests)
endif ()

install(TARGETS libdto DESTINATION lib)
install(FILES Dto.h ByteBuffer.h Bson.h Json.h Yaml.h DESTINATION include/libdto)