# default build type
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
endif()

#enabling ipo
if (ENABLE_IPO AND "${CMAKE_VERSION}" VERSION_GREATER 3.9)
	check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
	if (ipo_error)
		messsage("ipo error: ${ipo_error}")
	elseif(ipo_supported)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
	endif()
endif()
