# Force shader CRT for gtest
set(gtest_force_shared_crt ON)

# Add gtest library
add_subdirectory(googletest)

# Add tests executable
add_executable(dtotests
	Tests.cpp
	DtoTests.cpp
	EncoderTests.cpp
	IterTests.cpp
	BsonTests.cpp
	JsonTests.cpp
    YamlTests.cpp
    YamlJsonTests.cpp
    TokenizerTests.cpp
	)
	
# Add a source group
source_group("Code" FILES
	Tests.cpp
	DtoTests.cpp
	EncoderTests.cpp
	IterTests.cpp
	BsonTests.cpp
	JsonTests.cpp
    YamlTests.cpp
    YamlJsonTests.cpp
    TokenizerTests.cpp
	)

# Add include directories
include_directories(googletest/googletest/include)
include_directories(..)
	
# Link gtest and gtest_main libraries to tests target
target_link_libraries(dtotests libdto gtest gtest_main)
