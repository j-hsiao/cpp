add_executable(argtest test.cpp)
target_link_libraries(argtest PRIVATE argparse)

install(
	TARGETS argtest
	RUNTIME DESTINATION tests)
