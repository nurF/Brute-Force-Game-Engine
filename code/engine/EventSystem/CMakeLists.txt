CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
INCLUDE(USE)
#PROJECT(EventSystem)

USE_INIT_PACKAGE(EventSystem "1.0.0")

FIND_PACKAGE( Boost 1.40.0 REQUIRED date_time thread serialization iostreams system)

#message ("Found boost ${Boost_LIBRARY_DIRS} ${Boost_LIBRARIES}" )

FILE(GLOB Event_SOURCES         *.cpp)
FILE(GLOB EventCore_SOURCES		Core/*.cpp)
FILE(GLOB EventNetwork_SOURCES  Core/network/*.cpp)
FILE(GLOB EventSuite_SOURCES	EventSuite_Con/*.cpp)
FILE(GLOB EventTest_SOURCES		EventTypesTest/*.cpp)

FILE(GLOB Event_HEADERS         *.h)
FILE(GLOB EventCore_HEADERS		Core/*.h)
FILE(GLOB EventNetwork_HEADERS  Core/network/*.h )
FILE(GLOB EventSuite_HEADERS	EventSuite_Con/*.h)
FILE(GLOB EventTest_HEADERS		EventTypesTest/*.h)

SOURCE_GROUP("." FILES ${Event_SOURCES} ${Event_HEADERS})
SOURCE_GROUP("Core" FILES ${EventCore_SOURCES} ${EventCore_HEADERS})
SOURCE_GROUP("Test" FILES ${EventTest_SOURCES} ${EventTest_HEADERS})
SOURCE_GROUP("Network" FILES ${EventNetwork_SOURCES} ${EventNetwork_HEADERS})
SOURCE_GROUP("Suite" FILES ${EventSuite_SOURCES} ${EventSuite_HEADERS})

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} Core/ ../)
INCLUDE_DIRECTORIES(${BoostLog_INCLUDE_DIR})

ADD_LIBRARY(EventCore STATIC
	${Event_SOURCES}
	${Event_HEADERS}
	${EventCore_SOURCES}
	${EventCore_HEADERS}
	${EventNetwork_SOURCES}
	${EventNetwork_HEADERS}
)
TARGET_LINK_LIBRARIES(EventCore
	${Boost_LIBRARIES}
	${BoostLog_LIBRARIES}
	Base
)

INSTALL(TARGETS EventCore
		ARCHIVE DESTINATION ${USE_LIB_DIR}
		COMPONENT devel)

INSTALL(FILES ${EventCore_HEADERS}
		DESTINATION ${USE_PACKAGE_INCLUDE_DIR}
		COMPONENT devel)
		
INSTALL(FILES ${EventNetwork_HEADERS}
		DESTINATION "${USE_PACKAGE_INCLUDE_DIR}/network"
		COMPONENT devel)		
