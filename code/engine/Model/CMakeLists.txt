cmake_minimum_required(VERSION 2.6)

INCLUDE(USE)

USE_INIT_PACKAGE(Model "0.3.3")

FILE(GLOB Model_Root *.cpp *.h *.hh)
FILE(GLOB Model_Events Events/*.cpp Events/*.h)
FILE(GLOB Model_Loader Loader/*.cpp Loader/*.h)
FILE(GLOB Model_Model Model/*.cpp Model/*.h)
FILE(GLOB Model_Physics Physics/*.cpp Physics/*.h )
FILE(GLOB Model_Property Property/*.cpp Property/*.h)
FILE(GLOB Model_Concepts Property/Concepts/*.cpp Property/Concepts/*.h)

FILE(GLOB MODEL_FILES
	${Model_Root}
	${Model_Events}
	${Model_Loader}
	${Model_Model}
	${Model_Physics}
	${Model_Property}
	${Model_Concepts}
	*.xml
)

SOURCE_GROUP("." FILES ${Model_Root})
SOURCE_GROUP("Events" FILES ${Model_Events})
SOURCE_GROUP("Loader" FILES ${Model_Loader})
SOURCE_GROUP("Model" FILES ${Model_Model})
SOURCE_GROUP("Physics" FILES ${Model_Physics})
SOURCE_GROUP("Property" FILES ${Model_Property})
SOURCE_GROUP("Concepts" FILES ${Model_Concepts})

INCLUDE_DIRECTORIES(${BoostLog_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OGRE_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OIS_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${ODE_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${BOOST_GEOMETRY_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${MYGUI_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(..)
INCLUDE_DIRECTORIES(.)	# For subdirectories

ADD_DEFINITIONS(-DdSINGLE)
ADD_DEFINITIONS(-DTIXML_USE_STL)

IF(WIN32)
	ADD_DEFINITIONS(-DMODEL_EXPORTS)
ENDIF(WIN32)

ADD_CUSTOM_TARGET(ModelEnums 
	EnumGen --input ${CMAKE_CURRENT_SOURCE_DIR}/Enums.xml
	        --header ${CMAKE_CURRENT_SOURCE_DIR}/Enums.hh
	        --source ${CMAKE_CURRENT_SOURCE_DIR}/Enums.cpp
	DEPENDS EnumGen
)

ADD_LIBRARY(Model SHARED ${MODEL_FILES})

TARGET_LINK_LIBRARIES(Model
	${BoostLog_LIBRARIES}
	${Boost_LIBRARIES}
	TinyXML
	Base
	Controller
	EventCore
	Physics
	Core
	View
)

ADD_DEPENDENCIES(Model CoreEnums ModelEnums)

INSTALL(TARGETS Model
		LIBRARY DESTINATION ${USE_LIB_DIR}
		RUNTIME DESTINATION ${USE_LIB_DIR}
		COMPONENT devel
		)
		
INSTALL(FILES ${Model_Root}
		DESTINATION ${USE_PACKAGE_INCLUDE_DIR}
		COMPONENT devel)
		
INSTALL(FILES ${Model_Events}
		DESTINATION "${USE_PACKAGE_INCLUDE_DIR}/Events"
		COMPONENT devel)
		
INSTALL(FILES ${Model_Loader}
		DESTINATION "${USE_PACKAGE_INCLUDE_DIR}/Loader"
		COMPONENT devel)
		
INSTALL(FILES ${Model_Model}
		DESTINATION "${USE_PACKAGE_INCLUDE_DIR}/Model"
		COMPONENT devel)
		
INSTALL(FILES ${Model_Physics}
		DESTINATION "${USE_PACKAGE_INCLUDE_DIR}/Physics"
		COMPONENT devel)
		
INSTALL(FILES ${Model_PropertyConcepts}
		DESTINATION "${USE_PACKAGE_INCLUDE_DIR}/Model/PropertyConcept"
		COMPONENT devel)
		
