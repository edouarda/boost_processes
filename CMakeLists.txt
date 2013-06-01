cmake_minimum_required (VERSION 2.6)

enable_testing()

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(processes)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})

IF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")
    # we're building in 32 bit for sure
    SET(CUR_ARCH "32bit")
ELSE(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")
    IF(CMAKE_CL_64)
        SET(CUR_ARCH "64bit")
    ELSE(CMAKE_CL_64)
        IF (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            SET(CUR_ARCH "64bit")
        ELSE(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            SET(CUR_ARCH "32bit")
        ENDIF (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    ENDIF(CMAKE_CL_64)
ENDIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")

IF(CMAKE_COMPILER_IS_GNUCXX OR CLANG)
    find_library(Pthread_LIBRARY pthread)
    find_library(Realtime_LIBRARY rt)
    # find library can be problematic with stdc++ which is why we hardwire the link
    SET(Stdcpp_LIBRARY stdc++)
ELSE(CMAKE_COMPILER_IS_GNUCXX OR CLANG)
    SET(Pthread_LIBRARY "")
    SET(Realtime_LIBRARY "")
    SET(Stdcpp_LIBRARY "")
ENDIF(CMAKE_COMPILER_IS_GNUCXX OR CLANG)

IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    find_library(Kvm_LIBRARY kvm)
    find_library(Realtimecxx_LIBRARY cxxrt)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(Kvm_LIBRARY "")
    SET(Realtimecxx_LIBRARY "")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

IF (MSVC)
    IF(CUR_ARCH STREQUAL "64bit")
        MESSAGE(STATUS "Looking for 64-bit boost libraries")
        FILE(TO_CMAKE_PATH $ENV{BOOST_ROOT}/stage64/lib BOOST_LIBRARYDIR)
    ELSE(CUR_ARCH STREQUAL "64bit")
        MESSAGE(STATUS "Looking for 32-bit boost libraries")
        FILE(TO_CMAKE_PATH $ENV{BOOST_ROOT}/stage/lib BOOST_LIBRARYDIR)
    ENDIF(CUR_ARCH STREQUAL "64bit")

ENDIF(MSVC)

IF (CLANG)
    set(Boost_COMPILER "-clang32")
ENDIF (CLANG)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ADDITIONAL_VERSIONS 1.53.0)

find_package(Boost 1.53.0 COMPONENTS 
    chrono
    date_time
    filesystem
    prg_exec_monitor
    program_options
    system
    thread
    unit_test_framework
    test_exec_monitor
    regex
    serialization REQUIRED)

set(BOOST_PROCESSES_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(boost_processes STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/boost/processes/command_line.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/boost/processes/nexus.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/boost/processes/processes.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/boost/processes/scheduler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/boost/processes/structures.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/boost/processes/windows.hpp)
set_target_properties(boost_processes PROPERTIES LINKER_LANGUAGE CXX)


add_subdirectory(libs)