cmake_minimum_required(VERSION 3.7.0 FATAL_ERROR)
include(PrecompileHeader.cmake)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME GDLEnhanced)
# Output Variables
set(OUTPUT_DEBUG Bin/)
set(OUTPUT_REL Bin/)
# Folders files
set(CPP_DIR_1 Source)
set(CPP_DIR_2 Source/combat)
set(CPP_DIR_3 Source/command/admin)
set(CPP_DIR_4 Source/Messages/Client\ to\ Server)
set(CPP_DIR_5 Source/PhatSDK/Support)
set(CPP_DIR_6 Source/PhatSDK)
set(CPP_DIR_7 Source/weenie)

set(HEADER_DIR_1 Source)
set(HEADER_DIR_2 Source/combat)
set(HEADER_DIR_3 Source/Messages/Client\ to\ Server)
set(HEADER_DIR_4 Source/Messages)
set(HEADER_DIR_5 Source/PhatSDK)
set(HEADER_DIR_6 Source/PhatSDK/Support)
set(HEADER_DIR_7 Resource)
set(HEADER_DIR_8 Source/weenie)
#set(HEADER_DIR_9 Lib)

############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

option(GDLE_AVX "Enable SIMD/AVX" OFF)

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

# Definition of Macros
add_definitions(
   -D_DEBUG 
   -DNDEBUG
   -D__BUILDNAME__=DEBUG 
)

############## Artefacts Output #################
# Defines outputs , depending Debug or Release. #
#################################################

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
endif()

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
  add_definitions(-D_WINDOWS)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /W2 /EHsc")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W2 /EHsc")

  if (GDLE_AVX)
    add_compile_options("/arch:AVX")
  endif()
endif(MSVC)
if(NOT MSVC)
  add_definitions(-D_CONSOLE)
  set(CMAKE_EXE_LINKER_FLAGS "")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g2 -gz -O0")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-g1 -gz -O2")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -w")
  set(CMAKE_CXX_FLAGS_DEBUG "-g2 -gz -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-g1 -gz -O2")

  if (GDLE_AVX)
    add_compile_options("-mavx -mfma")
  endif()

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
  endif()
endif(NOT MSVC)

################ Files ################
#   --   Add files to project.   --   #
#######################################

file(GLOB SRC_FILES
    ${CPP_DIR_1}/*.cpp
    ${CPP_DIR_2}/*.cpp
    ${CPP_DIR_3}/*.cpp
    ${CPP_DIR_4}/*.cpp
    ${CPP_DIR_5}/*.cpp
    ${CPP_DIR_6}/*.cpp
    ${CPP_DIR_7}/*.cpp
    ${CPP_DIR_8}/*.cpp
#    ${HEADER_DIR_1}/*.h
#    ${HEADER_DIR_2}/*.h
#    ${HEADER_DIR_3}/*.h
#    ${HEADER_DIR_4}/*.h
#    ${HEADER_DIR_5}/*.h
#    ${HEADER_DIR_6}/*.h
#    ${HEADER_DIR_7}/*.h
#    ${HEADER_DIR_8}/*.h
    #${HEADER_DIR_9}/*.h
)

include_directories(
  Lib
  Resource
  Source
  Source/PhatSDK
  Source/PhatSDK/Support
  )

link_directories(${CMAKE_SOURCE_DIR}/Lib)

# Add executable to build.
add_executable(${PROJECT_NAME}
  ${SRC_FILES}
  ${CMAKE_SOURCE_DIR}/Resource/PhatAC.rc
)

add_precompiled_header(${PROJECT_NAME}
  Source/StdAfx.h
#  FORCEINCLUDE
  SOURCE_CXX Source/Precompiled.cpp)

if(MSVC)
  target_link_libraries(${PROJECT_NAME} ws2_32.lib comctl32.lib zlib/zlibstatDebug mysql/lib/libmysql )
endif(MSVC)
if(NOT MSVC)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)

  find_library(MYSQLLIB
    NAMES mysqlclient libmysqlclient libmysqlclient.so
    PATH_SUFFIXES mysql
  )

  if (MYSQLLIB STREQUAL "MYSQLLIB-NOTFOUND")
    message(FATAL_ERROR "mysqlclient not found")
  endif()

  # TODO: better handling of mysqlclient
  target_link_libraries(${PROJECT_NAME} Threads::Threads ${MYSQLLIB} z stdc++fs )
endif(NOT MSVC)
