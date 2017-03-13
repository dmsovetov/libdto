# Set the minimum CMake version
cmake_minimum_required(VERSION 3.2)

# Declare the project
project(libdto)

# Library source files
set(SRC
	Bson.cpp
	)
	
# Library header files
set(HEADERS
	Bson.h
	)
	
# Configure IDE source file filters
source_group("Code" FILES ${SRC} ${HEADERS})
	
# Add libdto static library
add_library(libdto STATIC ${SRC} ${HEADERS})

# Add a test executable
add_executable(dtotest Test.cpp)
target_link_libraries(dtotest libdto)
source_group("Code" FILES Test.cpp)

install(TARGETS libdto DESTINATION lib)
install(FILES Dto.h ByteBuffer.h Bson.h Json.h Yaml.h DESTINATION include/libdto)