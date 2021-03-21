cmake_minimum_required(VERSION 3.15...3.18)

project(IIModManager VERSION 0.1)


set(CMAKE_CXX_STANDARD 17)

set(IIMODMAN_VERSION ${IIModManager_VERSION})
set(IIMODMAN_SOVERSION 0.1.0)

option(BUILD_SHARED_LIBS "" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(IIMODMAN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IIMODMAN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(IIMODMAN_LIB_TARGET_NAME iimodman)
set(IIMODMAN_CLI_TARGET_NAME iimodman-cli)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(QUAZIP_QT_MAJOR_VERSION 6)
add_subdirectory(quazip)

add_subdirectory(iimodman-cli)
add_subdirectory(iimodman-lib)
