cmake_minimum_required(VERSION 3.15...3.18)

project(IIModManager VERSION 0.1)


set(CMAKE_CXX_STANDARD 17)

set(IIMODMAN_VERSION ${IIModManager_VERSION})
set(IIMODMAN_SOVERSION 0.1.0)

option(BUILD_SHARED_LIBS "" OFF)
set(IIMODMAN_QT_MAJOR_VERSION 5 CACHE STRING "Qt version to use, defaults to 5")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(IIMODMAN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IIMODMAN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(IIMODMAN_LIB_TARGET_NAME iimodman)
set(IIMODMAN_CLI_TARGET_NAME iimodman-cli)

if(IIMODMAN_QT_MAJOR_VERSION EQUAL 6)
        find_package(Qt6 REQUIRED COMPONENTS Core Network REQUIRED)
        set(IIMODMAN_LIB_QT_LIBRARIES Qt6::Core Qt6::Network)
        set(IIMODMAN_CLI_QT_LIBRARIES Qt6::Core)
elseif(IIMODMAN_QT_MAJOR_VERSION EQUAL 5)
        find_package(Qt5 REQUIRED COMPONENTS Core Network REQUIRED)
        set(IIMODMAN_LIB_QT_LIBRARIES Qt5::Core Qt5::Network)
        set(IIMODMAN_CLI_QT_LIBRARIES Qt5::Core)
else()
        message(FATAL_ERROR "Qt version ${IIMODMAN_QT_MAJOR_VERSION} is not supported")
endif()

set(QUAZIP_QT_MAJOR_VERSION ${IIMODMAN_QT_MAJOR_VERSION})
add_subdirectory(quazip)

add_subdirectory(iimodman-cli)
add_subdirectory(iimodman-lib)
