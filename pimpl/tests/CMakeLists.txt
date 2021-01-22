add_executable(pimpltest ./test.cpp)
target_link_libraries(pimpltest PRIVATE pimpl)
install(
	TARGETS pimpltest
	RUNTIME DESTINATION tests
)
