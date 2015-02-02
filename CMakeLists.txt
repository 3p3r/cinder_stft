# Project Properties
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

# Project code-name and linker language
PROJECT (ciEq CXX)

# Project name, version, and etc.
SET (CIEQ_NAME "Cinder Equalizer")
SET (CIEQ_VERSION_MAJOR 1)
SET (CIEQ_VERSION_MINOR 0)
SET (CIEQ_VERSION_PATCH 0)
SET (CIEQ_VERSION_STRING "${CIEQ_VERSION_MAJOR}-${CIEQ_VERSION_MINOR}-${CIEQ_VERSION_PATCH}")

# debug option
SET (CMAKE_VERBOSE_MAKEFILE 1)

# Cinder root
SET (CINDER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cinder")

# we only support MSVC, so check for it.
IF (NOT MSVC)

MESSAGE(FATAL_ERROR "This CMakeLists was designed to build this project \
under Windows and with Visual Studio only. Try builiding manually.")

ENDIF(NOT MSVC)

# Check if Cinder is cloned
IF (NOT EXISTS "${CINDER_ROOT}")

MESSAGE(FATAL_ERROR "Could not find Cinder. Initialize and build \
Cinder via git submodules first. Consult README.md for guide.")

ENDIF(NOT EXISTS "${CINDER_ROOT}")

# Check if Cinder dependencies are cloned
IF (NOT EXISTS "${CINDER_ROOT}/boost/boost/version.hpp")

MESSAGE(FATAL_ERROR "Could not find Cinder dependencies. Initialize \
Cinder dependecies via git submodules first. Consult README.md for guide.")

ENDIF(NOT EXISTS "${CINDER_ROOT}/boost/boost/version.hpp")

# look for Cinder static libraries
FILE(GLOB CINDER_RELEASE_LIB "${CINDER_ROOT}/lib/msw/x86/cinder-*[!d].lib")
FILE(GLOB CINDER_DEBUG_LIB "${CINDER_ROOT}/lib/msw/x86/cinder-*[_d].lib")

# check if Cinder debug library is found
IF ("${CINDER_DEBUG_LIB}" STREQUAL "")

MESSAGE(FATAL_ERROR "Could not find Cinder debug library. Build Cinder \
debug static library first. Consult README.md for guide.")

ENDIF("${CINDER_DEBUG_LIB}" STREQUAL "")

# check if Cinder release library is found
IF ("${CINDER_RELEASE_LIB}" STREQUAL "")

MESSAGE(FATAL_ERROR "Could not find Cinder release library. Build Cinder \
release static library first. Consult README.md for guide.")

ENDIF("${CINDER_RELEASE_LIB}" STREQUAL "")

# appropriately set the Cinder lib variable for linking
SET(CINDER_LIBRARY optimized ${CINDER_RELEASE_LIB} debug ${CINDER_DEBUG_LIB})

# Look for all sources that will participate in this build session
FILE(GLOB_RECURSE CIEQ_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
FILE(GLOB_RECURSE CIEQ_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

INCLUDE_DIRECTORIES(
	"${CINDER_ROOT}/include"
	"${CINDER_ROOT}/boost"
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

ADD_EXECUTABLE(ciEq ${CIEQ_HEADERS} ${CIEQ_SOURCES})

# Link shit together
TARGET_LINK_LIBRARIES(ciEq ${CINDER_LIBRARY})

# Added version string to the executable in build
SET_TARGET_PROPERTIES(ciEq PROPERTIES SUFFIX "_v${CIEQ_VERSION_STRING}")

# Added a "d" suffix to the executable in debug build
SET_TARGET_PROPERTIES(ciEq PROPERTIES DEBUG_POSTFIX "d")