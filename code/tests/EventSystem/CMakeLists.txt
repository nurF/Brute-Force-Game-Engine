# --------------------------- preamble ----------
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
ENABLE_TESTING()

# --------------------------- includes ----------
INCLUDE(USE)

# --------------------------- package declaration
USE_INIT_PACKAGE(EventTest "1.0.0")


# --------------------------- dependencies ------
FIND_PACKAGE( Boost 1.40.0 REQUIRED date_time thread serialization iostreams system)


# --------------------------- test sources ------
FILE(GLOB EventTest_SOURCES		*.cpp)
FILE(GLOB EventTest_HEADERS		*.h)


# --------------------------- IDE settings ------
SOURCE_GROUP("." FILES ${EventTest_SOURCES} ${EventTest_HEADERS})

# --------------------------- Setting up the build system
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${BoostLog_INCLUDE_DIR})

#temporars
SET(ENGINE_DIR "../../engine") # need findEngine.cmake?

INCLUDE_DIRECTORIES(${ENGINE_DIR})
# --------------------------- Targets -----------
ADD_EXECUTABLE(EventTypesTest ${EventTest_SOURCES})
TARGET_LINK_LIBRARIES(EventTypesTest ${Boost_LIBRARIES} EventCore)


# --------------------------- Tests-- -----------
ADD_TEST(TypesTest EventTypesTest)


#ADD_EXECUTABLE(EventSuite_Con ${EventSuite_SOURCES})
#TARGET_LINK_LIBRARIES(EventSuite_Con ${Boost_LIBRARIES} EventCore)

#ADD_EXECUTABLE(EventTypesTest ${EventTest_SOURCES})
#TARGET_LINK_LIBRARIES(EventTypesTest ${Boost_LIBRARIES} EventCore)

#INSTALL(TARGETS EventCore
#		ARCHIVE DESTINATION ${USE_LIB_DIR}
#		COMPONENT devel)

#INSTALL(TARGETS EventSuite_Con EventTypesTest
#		RUNTIME DESTINATION ${USE_BIN_DIR}
#		COMPONENT runtime)
		
#INSTALL(FILES ${EventCore_HEADERS}
#		DESTINATION ${USE_PACKAGE_INCLUDE_DIR}
#		COMPONENT devel)
		
#INSTALL(FILES ${EventNetwork_HEADERS}
#		DESTINATION "${USE_PACKAGE_INCLUDE_DIR}/network"
#		COMPONENT devel)		
