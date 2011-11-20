FILE (GLOB ODE_SOURCES	../ode/src/*.cpp
						../ode/src/*.c
						../ode/src/joints/*.cpp
						../OPCODE/*.cpp
						../OPCODE/Ice/*.cpp)
FILE (GLOB ODE_HEADERS	../ode/src/*.h
						../ode/src/joints/*.h
						../OPCODE/*.h
						../OPCODE/Ice.h)

ADD_DEFINITIONS(-DdSINGLE -D__ODE__)
INCLUDE_DIRECTORIES(../include/)
INCLUDE_DIRECTORIES(../ode/src/)
INCLUDE_DIRECTORIES(../OPCODE/)
INCLUDE_DIRECTORIES(./)

ADD_LIBRARY(ode STATIC ${ODE_SOURCES} ${ODE_HEADERS})
