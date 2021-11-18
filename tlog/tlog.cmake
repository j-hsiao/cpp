# TLOG_NS: the namespace to put Cerr into
# TLOG_DEBUG: the name of the project's debug variable
# output: the output file path.
#	defaults to CMAKE_CURRENT_BINARY_DIR/include/PROJECT_NAME/tlog.hpp

if(CMAKE_VERSION VERSION_LESS 3.17)
	set(TLOG_HPP_IN "${CMAKE_CURRENT_LIST_DIR}/tlog.hpp.in")
endif()
function(make_tlog TLOG_NS TLOG_DEBUG)
	if (ARGC GREATER 2)
		set(output "${ARGV2}")
	else()
		set(
			output
			"${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/tlog.hpp")
	endif()
	if(CMAKE_VERSION VERSION_LESS 3.17)
		configure_file("${TLOG_HPP_IN}" "${output}")
	else()
		configure_file(
			"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/tlog.hpp.in" "${output}")
	endif()
endfunction()
