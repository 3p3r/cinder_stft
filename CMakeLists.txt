# Project Properties
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

# We only need debug / release configurations
# For some reason this should be here before PROJECT(..)
SET(CMAKE_CONFIGURATION_TYPES Debug Release)

# Project code-name and linker language
PROJECT (ciEq CXX)

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

LINK_DIRECTORIES(
	"${CINDER_ROOT}/lib/msw"
	"${CINDER_ROOT}/lib/msw/x86"
)

ADD_EXECUTABLE(ciEq WIN32 ${CIEQ_HEADERS} ${CIEQ_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/Resources.rc")

# Link shit together
TARGET_LINK_LIBRARIES(ciEq ${CINDER_LIBRARY})

# Added a "d" suffix to the executable in debug build
SET_TARGET_PROPERTIES(ciEq PROPERTIES DEBUG_POSTFIX "d")

# set character set to Unicode
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

# take out Windows obsolete headers
ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
ADD_DEFINITIONS(-DVC_EXTRALEAN)
ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)

# set to MT[d] flag for build (Cinder does not like MD[d])
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

# exclude libcmt from debug build. Cinder doesn't like it
SET_TARGET_PROPERTIES(ciEq PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:\"libcmt\"")