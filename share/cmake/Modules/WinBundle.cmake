
MACRO(SETUP_BUNDLE_PATHS)

	SET(BFG_THIRDPARTY_DIR "${PROJECT_SOURCE_DIR}/thirdparty")

	# ----- #
	# Boost #
	# ----- #

	SET(BOOST_ROOT "${BFG_THIRDPARTY_DIR}/boost_1_47_0")
	SET(Boost_USE_MULTITHREADED TRUE)
	SET(Boost_USE_STATIC_LIBS   FALSE)

	# --------- #
	# Boost.Log #
	# --------- #

	SET(BoostLog_INCLUDE_DIR "${BFG_THIRDPARTY_DIR}/boost-log")
	SET(BoostLog_LIBRARY_DIR "${BFG_THIRDPARTY_DIR}/boost-log/lib")

	# CMake <=> Bjam Mapping for Visual Studio versions
	IF    (${MSVC_VERSION} EQUAL "1500")
		SET(BJAM_MS_INTERFIX "vc90")
	ELSEIF(${MSVC_VERSION} EQUAL "1600")
		SET(BJAM_MS_INTERFIX "vc100")
	ELSE  (${MSVC_VERSION} EQUAL "1500")
		SET(BJAM_MS_INTERFIX "" STRING)
		MESSAGE(WARNING
			"Unknown Microsoft compiler detected, so you'll have"
			" to set the correct interfix (e.g. \"vs110\") manually."
		)
	ENDIF(${MSVC_VERSION} EQUAL "1500")

	IF(CMAKE_CONFIGURATION_TYPES)
		SET(BoostLog_LIBRARIES
			debug ${BoostLog_LIBRARY_DIR}/boost_log_setup-${BJAM_MS_INTERFIX}-mt-gd-1_47.lib
			debug ${BoostLog_LIBRARY_DIR}/boost_log-${BJAM_MS_INTERFIX}-mt-gd-1_47.lib
			optimized ${BoostLog_LIBRARY_DIR}/boost_log_setup-${BJAM_MS_INTERFIX}-mt-1_47.lib
			optimized ${BoostLog_LIBRARY_DIR}/boost_log-${BJAM_MS_INTERFIX}-mt-1_47.lib
		)
	ELSE(CMAKE_CONFIGURATION_TYPES)
		IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
			SET(BoostLog_LIBRARIES
				${BoostLog_LIBRARY_DIR}/boost_log_setup-${BJAM_MS_INTERFIX}-mt-gd-1_47.lib
				${BoostLog_LIBRARY_DIR}/boost_log-${BJAM_MS_INTERFIX}-mt-gd-1_47.lib
			)
		ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug") # Release!
			SET(BoostLog_LIBRARIES
				${BoostLog_LIBRARY_DIR}/boost_log_setup-${BJAM_MS_INTERFIX}-mt-1_47.lib
				${BoostLog_LIBRARY_DIR}/boost_log-${BJAM_MS_INTERFIX}-mt-1_47.lib
			)
		ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	ENDIF(CMAKE_CONFIGURATION_TYPES)

	# -------#
	# OpenAL #
	# -------#

	SET(BoostLog_INCLUDE_DIR "${BFG_THIRDPARTY_DIR}/boost-log")
	SET(OPENAL_INCLUDE_DIR ${BFG_THIRDPARTY_DIR}/OpenAL/include)
	SET(OPENAL_LIBRARY_DIR ${BFG_THIRDPARTY_DIR}/OpenAL/libs/Win32)
	SET(OPENAL_LIBRARY ${OPENAL_LIBRARY_DIR}/OpenAL32.lib)

ENDMACRO(SETUP_BUNDLE_PATHS)

