#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=avr-gcc
CCC=avr-g++
CXX=avr-g++
FC=gfortran
AS=avr-as

# Macros
CND_PLATFORM=Arduino-1.6.9-Windows
CND_DLIB_EXT=dll
CND_CONF=UNO-Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/UARTApp1.o


# C Compiler Flags
CFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -DARDUINO=${ARDUINO} -Wextra -flto -g -Os -ffunction-sections -fdata-sections

# CC Compiler Flags
CCFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -D${VARIANT} -DARDUINO=${ARDUINO} -fno-exceptions -Wextra -flto -std=gnu++11 -felide-constructors -Os -ffunction-sections -fdata-sections -mcall-prologues
CXXFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -D${VARIANT} -DARDUINO=${ARDUINO} -fno-exceptions -Wextra -flto -std=gnu++11 -felide-constructors -Os -ffunction-sections -fdata-sections -mcall-prologues

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../../dist/UNO-Release/Arduino-1.6.9-Windows/libfastarduino.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp1.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp1.exe: ../../../dist/UNO-Release/Arduino-1.6.9-Windows/libfastarduino.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp1.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp1 ${OBJECTFILES} ${LDLIBSOPTIONS} -Os -Wl,--gc-sections -Wl,--relax -flto -mmcu=${MCU} -Wl,-Map,${CND_ARTIFACT_PATH_${CONF}}.map

${OBJECTDIR}/UARTApp1.o: UARTApp1.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -DARDUINO_UNO -I../../../cores -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UARTApp1.o UARTApp1.cpp

# Subprojects
.build-subprojects:
	cd ../../.. && ${MAKE}  -f Makefile CONF=UNO-Release

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp1.exe

# Subprojects
.clean-subprojects:
	cd ../../.. && ${MAKE}  -f Makefile CONF=UNO-Release clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
