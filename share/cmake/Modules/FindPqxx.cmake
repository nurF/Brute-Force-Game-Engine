IF (Pqxx_INCLUDE_DIR AND Pqxx_LIBRARIES)
	SET (Pqxx_FOUND TRUE)
ELSE ()
	FIND_PATH (Pqxx_INCLUDE_DIR pqxx/pqxx
		/usr/include
		/usr/local/include
		${PQXX_ROOT}/include
	)
	# on 64 bit linux /usr/lib64 is a symlink to /usr/lib
	FIND_LIBRARY (Pqxx_LIBRARIES NAMES pqxx libpqxx
		PATHS
		/usr/lib
		/usr/local/lib
		${PQXX_ROOT}/lib
	)
	
	IF (Pqxx_INCLUDE_DIR AND Pqxx_LIBRARIES)
		SET (Pqxx_FOUND TRUE)
		MESSAGE (STATUS "Found libpqxx: ${Pqxx_INCLUDE_DIR}, ${Pqxx_LIBRARIES}")
	ELSE ()
		SET (Pqxx_FOUND FALSE)
		MESSAGE(STATUS "libpqxx not found.")
	ENDIF ()
	
	MARK_AS_ADVANCED(Pqxx_INCLUDE_DIR Pqxx_LIBRARIES)
ENDIF ()
