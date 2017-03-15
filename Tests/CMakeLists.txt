add_subdirectory(gtest)

# Add tests executable
add_executable(tests
	Tests.cpp
	)
	
# Add a source group
source_group("Code" FILES Tests.cpp)
	
# Link gtest and gtest_main libraries to tests target
target_link_libraries(dtotests libdto gtest gtest_main)