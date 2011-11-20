IF (BoostLog_INCLUDE_DIR AND BoostLog_LIBRARIES)
	SET (BoostLog_FOUND TRUE)
ELSE ()
	# only useful with unix system and correctly installed boost log
	# on windows the paths should be set manually
	FIND_PATH (BoostLog_INCLUDE_DIR boost/log/trivial.hpp
		/usr/include
		/usr/local
		${BOOSTLOG_ROOT}/include
		${BOOST_ROOT}/include
		${BOOSTROOT}/include
	)
	# on 64 bit linux /usr/lib64 is a symlink to /usr/lib
	FIND_LIBRARY (BoostLog_LIBRARIES NAMES boost_log libboost_log
		PATHS
		/usr/lib
		/usr/local/lib
		${BOOSTLOG_ROOT}/lib
		${BOOST_ROOT}/lib
		${BOOSTROOT}/lib
	)
	
	IF (BoostLog_INCLUDE_DIR AND BoostLog_LIBRARIES)
		SET (BoostLog_FOUND TRUE)
		MESSAGE (STATUS "Found Boost.Log: ${BoostLog_INCLUDE_DIR}, ${BoostLog_LIBRARIES}")
	ELSE ()
		SET (BoostLog_FOUND FALSE)
		MESSAGE(STATUS "Boost.Log not found.")
	ENDIF ()
	
	MARK_AS_ADVANCED(BoostLog_INCLUDE_DIR BoostLog_LIBRARIES)
ENDIF ()
