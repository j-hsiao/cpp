add_executable(timeutil_test ./timeutil.cpp)
target_link_libraries(timeutil_test PRIVATE timeutil)
install(
	TARGETS timeutil_test
	RUNTIME DESTINATION tests)
