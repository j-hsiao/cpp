add_executable(getcpuinfo ./main.cpp)
target_link_libraries(getcpuinfo PRIVATE cpuinfo)
install(
	TARGETS getcpuinfo
	RUNTIME DESTINATION tests
)
