cmake_minimum_required(VERSION 2.6)

INCLUDE(USE)

USE_INIT_PACKAGE(View "0.5.0")

FILE(GLOB View_SOURCES *.cpp)
FILE(GLOB View_HEADERS *.h *.xml)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OGRE_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${MYGUI_OGRE_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${MYGUI_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${BoostLog_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${BOOST_GEOMETRY_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OIS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(..)

ADD_DEFINITIONS(-DVIEW_EXPORTS)

ADD_CUSTOM_TARGET(ViewEnums
	EnumGen --input ${CMAKE_CURRENT_SOURCE_DIR}/Enums.xml
	        --header ${CMAKE_CURRENT_SOURCE_DIR}/Enums.hh
	        --source ${CMAKE_CURRENT_SOURCE_DIR}/Enums.cpp
	DEPENDS EnumGen
)

ADD_LIBRARY(View SHARED
	${View_SOURCES}
	${View_HEADERS}
)
TARGET_LINK_LIBRARIES(View
	${Boost_LIBRARIES}
	${BoostLog_LIBRARIES}
	${OGRE_LIBRARIES}
	${MYGUI_LIBRARIES}
	${MYGUI_OGRE_LIBRARIES}
	${OIS_LIBRARIES}
	Base
	Controller
	EventCore
	Core
)

ADD_DEPENDENCIES(View Core Controller EventCore EnumGen ViewEnums)

INSTALL(TARGETS View
		LIBRARY DESTINATION ${USE_LIB_DIR}
		RUNTIME DESTINATION ${USE_LIB_DIR}
		COMPONENT runtime)
		
INSTALL(FILES ${View_HEADERS}
		DESTINATION ${USE_PACKAGE_INCLUDE_DIR}
		COMPONENT devel)
