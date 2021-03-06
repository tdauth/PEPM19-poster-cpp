cmake_minimum_required(VERSION 3.0.0)
project(PEPM19-poster-cpp)

set(boost_VERSION "1.66.0")
set(boost_VERSION_UNDERSCORE "1_66_0")
set(folly_VERSION "v2018.11.26.00")

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG(-std=c++17 HAS_CXX_17)
CHECK_CXX_COMPILER_FLAG(-std=c++14 HAS_CXX_14)
CHECK_CXX_COMPILER_FLAG(-std=c++11 HAS_CXX_11)

# Try to use standard C++17 but if it is not available fallback until C++11.
# The environment variables are required for the Bash scripts which often compile external libraries.
if (HAS_CXX_17)
	add_compile_options(-std=c++1z)
	set(ENV{CFLAGS} "-std=c11")
	set(ENV{CXXFLAGS} "-std=c++17")
elseif (HAS_CXX_14)
	add_compile_options(-std=c++14)
	set(ENV{CFLAGS} "-std=c11")
	set(ENV{CXXFLAGS} "-std=c++14")
elseif (HAS_CXX_11)
	add_compile_options(-std=c++11)
	set(ENV{CFLAGS} "-std=c11")
	set(ENV{CXXFLAGS} "-std=c++11")
endif ()

add_compile_options(-Wall)
# These flags are required for Boost.Thread and Boost.Test:
add_definitions(-DBOOST_THREAD_VERSION=4 -DBOOST_THREAD_PROVIDES_EXECUTORS -DBOOST_TEST_DYN_LINK)
# Workaround for Fedora:
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")

# Bash is required for many of the build scripts for the external projects.
find_program(BASH_PATH bash)

if (NOT BASH_PATH)
	message(FATAL_ERROR "bash not found.")
endif ()

include(ExternalProject)

# Boost
set(BOOST_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/boost-prefix/src/boost/")
set(BOOST_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/boost_install")
set(BOOST_INCLUDE_DIR "${BOOST_INSTALL_DIR}/include")
set(BOOST_LIB_DIR "${BOOST_INSTALL_DIR}/lib")

ExternalProject_Add(
	boost
	URL "https://sourceforge.net/projects/boost/files/boost/${boost_VERSION}/boost_${boost_VERSION_UNDERSCORE}.tar.bz2"
	INSTALL_COMMAND "" # Disable install step
	UPDATE_COMMAND "" # Doesn't change often
	CONFIGURE_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/configureboost.sh" "${BOOST_DOWNLOAD_DIR}" "${BOOST_INSTALL_DIR}"
	BUILD_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/buildboost.sh" "${BOOST_DOWNLOAD_DIR}" "${BOOST_LIB_DIR}"
	LOG_DOWNLOAD ON
	LOG_CONFIGURE ON
	LOG_BUILD ON
)

ExternalProject_Add_StepDependencies(boost configure "${CMAKE_CURRENT_SOURCE_DIR}/configureboost.sh")
ExternalProject_Add_StepDependencies(boost build "${CMAKE_CURRENT_SOURCE_DIR}/buildboost.sh")

include_directories("${BOOST_INCLUDE_DIR}")
link_directories("${BOOST_LIB_DIR}")

set(Boost_LIBRARIES
	"${BOOST_LIB_DIR}/libboost_thread.so"
	"${BOOST_LIB_DIR}/libboost_system.so"
	"${BOOST_LIB_DIR}/libboost_chrono.so"
	"${BOOST_LIB_DIR}/libboost_unit_test_framework.so"
	# For Folly
	# context, regex, filesystem and program_options are required by Folly
	"${BOOST_LIB_DIR}/libboost_context.so"
	"${BOOST_LIB_DIR}/libboost_regex.so"
	"${BOOST_LIB_DIR}/libboost_filesystem.so"
	"${BOOST_LIB_DIR}/libboost_program_options.so"
)

# Folly

# See https://github.com/facebook/folly/blob/master/README.md for all dependencies which have to be installed on Fedora as well.
# We have to find several dependencies manually and link them with the static Folly library.

find_library(GLOG_LIBRARY glog)

if (NOT GLOG_LIBRARY)
	message(FATAL_ERROR "Library glog not found.")
endif ()

find_library(GFLAGS_LIBRARY gflags)

if (NOT GFLAGS_LIBRARY)
	message(FATAL_ERROR "Library gflags not found.")
endif ()

find_library(DL_LIBRARY dl)

if (NOT DL_LIBRARY)
	message(FATAL_ERROR "Library dl not found.")
endif ()

find_library(EVENT_LIBRARY event)

if (NOT EVENT_LIBRARY)
	message(FATAL_ERROR "Library event not found.")
endif ()

find_library(EVENT_CORE_LIBRARY event_core)

if (NOT EVENT_CORE_LIBRARY)
	message(FATAL_ERROR "Library event_core not found.")
endif ()

find_library(DOUBLE_CONVERSION_LIBRARY double-conversion)

if (NOT DOUBLE_CONVERSION_LIBRARY)
	message(FATAL_ERROR "Library double-conversion not found.")
endif ()

find_library(PTHREAD_LIBRARY pthread)

if (NOT PTHREAD_LIBRARY)
	message(FATAL_ERROR "Library pthread not found.")
endif ()

set(folly_DIR "${CMAKE_CURRENT_BINARY_DIR}/folly-prefix/src/folly")
set(folly_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/folly_install")
set(folly_BUILD_DIR "${CMAKE_BINARY_DIR}/folly-prefix/src/folly-build")
set(folly_INCLUDE_DIR "${folly_INSTALL_DIR}/include")
set(folly_LIB_DIR "${folly_INSTALL_DIR}/lib")

set(folly_LIBRARIES
	"${folly_LIB_DIR}/libfolly.a"
	"${folly_LIB_DIR}/libfollybenchmark.a"
	${GLOG_LIBRARY}
	${GFLAGS_LIBRARY}
	${DL_LIBRARY}
	${EVENT_LIBRARY}
	${EVENT_CORE_LIBRARY}
	${DOUBLE_CONVERSION_LIBRARY}
	${PTHREAD_LIBRARY}
)

# The Boost libraries have to be linked as well with the static Folly library.
list(APPEND folly_LIBRARIES ${Boost_LIBRARIES})

ExternalProject_Add(folly
	# TODO Use the latest release where the bug fix is included: https://github.com/facebook/folly/commit/e9842c1e40d1d3e2a1fac9788e14492055855ea3
	#URL "https://github.com/facebook/folly/archive/${folly_VERSION}.tar.gz"
	GIT_REPOSITORY "https://github.com/facebook/folly.git"
	INSTALL_COMMAND ""
	CONFIGURE_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/configurefolly.sh" "${folly_DIR}" "${folly_INSTALL_DIR}" "${BOOST_INSTALL_DIR}"
	BUILD_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/buildfolly.sh" "${folly_BUILD_DIR}"
	DEPENDS boost
)

include_directories("${folly_INCLUDE_DIR}")
link_directories("${folly_LIB_DIR}")

add_subdirectory(src)
