# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Main CMake file
#
################################################################################

cmake_minimum_required(VERSION 3.21)

# CMake setup ##################################################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Generic builds.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Multi-config builds.
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
endforeach()

# Global variables #############################################################
# Executables.
set(EXECUTABLE_NAME "comet_editor")
set(TESTS_EXECUTABLE_NAME "comet_tests")

# Internal library names.
set(COMET_LIBRARY_NAME "comet")

# Other.
set(MESSAGE_PREFIX "[Comet] ")
set(VCPKG_REPO "https://github.com/Microsoft/vcpkg.git")

# Build type.
if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "${MESSAGE_PREFIX}Build type: ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(COMET_DEBUG)
endif()

# OS specifics #################################################################
if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

# Vcpkg setup ##################################################################
string(TOLOWER ${CMAKE_BUILD_TYPE} VCPKG_BUILD_TYPE)
message(STATUS "${MESSAGE_PREFIX}Vcpkg Build type: ${CMAKE_BUILD_TYPE}")
set(ENV{VCPKG_FEATURE_FLAGS} "versions")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(NOT DEFINED VCPKG_ROOT)
    if(NOT DEFINED ENV{VCPKG_ROOT})
      message(FATAL_ERROR
        "${MESSAGE_PREFIX}CMAKE_TOOLCHAIN_FILE and VCPKG_ROOT are not set. "
        "Please define at least VCPKG_ROOT to your Vcpkg root directory. "
        "Aborting.")
    else()
      set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    endif()
  endif()

  if(NOT EXISTS ${VCPKG_ROOT})
    if(NOT DEFINED IS_VCPKG_CLONE OR NOT IS_VCPKG_CLONE)
      message(FATAL_ERROR
        "${MESSAGE_PREFIX}Vcpkg does not exist at ${VCPKG_ROOT} and boolean "
        "IS_VCPKG_CLONE is not set to TRUE. Aborting.")
    endif()

    message(STATUS "${MESSAGE_PREFIX}Cloning Vcpkg in ${VCPKG_ROOT}...")
    execute_process(COMMAND git clone ${VCPKG_REPO} ${VCPKG_ROOT})
  else()
    message(STATUS "${MESSAGE_PREFIX}Updating Vcpkg at ${VCPKG_ROOT}...")
    execute_process(COMMAND git pull --ff-only WORKING_DIRECTORY ${VCPKG_ROOT})
  endif()

  if(WIN32)
    set(VCPKG_BIN ${VCPKG_ROOT}/vcpkg.exe)
    set(VCPKG_BOOTSTRAP_SCRIPT ${VCPKG_ROOT}/bootstrap-vcpkg.bat)
  else()
    set(VCPKG_BIN ${VCPKG_ROOT}/vcpkg)
    set(VCPKG_BOOTSTRAP_SCRIPT ${VCPKG_ROOT}/bootstrap-vcpkg.sh)
  endif()

  if(NOT EXISTS ${VCPKG_BIN})
    message(STATUS "${MESSAGE_PREFIX}Bootstrapping Vcpkg in ${VCPKG_ROOT}...")
    execute_process(
      COMMAND ${VCPKG_BOOTSTRAP_SCRIPT} WORKING_DIRECTORY ${VCPKG_ROOT}
    )
  endif()

  if(NOT EXISTS ${VCPKG_BIN})
    message(FATAL_ERROR
      "${MESSAGE_PREFIX}Something wrong happened while boostrapping Vcpkg. "
      "Aborting.")
  endif()

  if(WIN32)
    execute_process(
      COMMAND ${VCPKG_BIN} integrate install
    )
  endif()

  if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE
      "${VCPKG_ROOT}\\scripts\\buildsystems\\vcpkg.cmake")
  else()
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  endif()

  message(STATUS "${MESSAGE_PREFIX}CMAKE_TOOLCHAIN_FILE set to"
    " \"${CMAKE_TOOLCHAIN_FILE}\".")
endif()

# CMAKE_TOOLCHAIN_FILE might contain backslashes as path separators,
# which can be interpreted as escape characters...
if(WIN32)
  file(TO_CMAKE_PATH "${CMAKE_TOOLCHAIN_FILE}" CMAKE_TOOLCHAIN_FILE)
endif()

# Project ######################################################################

message(STATUS
  "${MESSAGE_PREFIX}Building ${CMAKE_PROJECT_NAME} "
  "to ${CMAKE_CURRENT_BINARY_DIR}...")

project("comet_game_engine"
  VERSION 0.1.0
  DESCRIPTION "Simple video game engine"
  LANGUAGES CXX ASM ASM_MASM
)

option(COMET_ARE_WARNINGS_ERRORS "Treat warnings as errors" OFF)
option(COMET_IS_TSAN "Enable TSan (ThreadSanitizer)" OFF)
option(COMET_IS_ASAN "Enable ASan (AddressSanitizer)" OFF)
option(COMET_ARE_STACK_OVERFLOW_CHECKS "Enable stack overflow checks" ON)
option(COMET_IS_BUILD_PERFORMANCE_SUMMARY "Request a build performance summary (MSVC only)" OFF)

if(COMET_ARE_STACK_OVERFLOW_CHECKS)
  add_compile_definitions(COMET_CHECK_STACK_OVERFLOWS)
endif()

if(COMET_IS_ASAN)
  add_compile_definitions(COMET_IS_ASAN)
endif()

if(COMET_IS_TSAN)
  add_compile_definitions(COMET_IS_TSAN)
endif()

# Compiler specifics.
if(MSVC)
  # Disable any non-conformant code with Microsoft Visual C++.
  # As to why W4 is used: https://stackoverflow.com/questions/4001736/whats-up-with-the-thousands-of-warnings-in-standard-headers-in-msvc-wall#comment8922238_4001759
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /W4")

  if(COMET_ARE_WARNINGS_ERRORS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
  endif()

  if(COMET_IS_ASAN)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    SET(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} /fsanitize=address")
    add_definitions(-D_DISABLE_STRING_ANNOTATION=1)
    add_definitions(-D_DISABLE_VECTOR_ANNOTATION=1)
  endif()

  if(COMET_IS_TSAN)
    message(ERROR "${MESSAGE_PREFIX}Unsupported option on current compiler: COMET_IS_TSAN")
  endif()
else()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -mavx")

  if(COMET_ARE_WARNINGS_ERRORS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic")
  endif()

  if(COMET_IS_ASAN)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    SET(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
  endif()

  if(COMET_IS_TSAN)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
    SET(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
  endif()
endif()

if(COMET_DEBUG_RELEASE)
  if(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    add_link_options("/INCREMENTAL:NO" "/DEBUG" "/OPT:REF" "/OPT:ICF")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -rdynamic -DNDEBUG")
  endif()
endif()

set(COMET_NAME ${CMAKE_PROJECT_NAME})
string(REPLACE "." ";" VERSION_LIST "${PROJECT_VERSION}")
list(GET VERSION_LIST 0 COMET_VERSION_MAJOR)
list(GET VERSION_LIST 1 COMET_VERSION_MINOR)
list(GET VERSION_LIST 2 COMET_VERSION_PATCH)

add_library(${COMET_LIBRARY_NAME})

target_precompile_headers(${COMET_LIBRARY_NAME}
  PRIVATE 
    "${PROJECT_SOURCE_DIR}/src/comet_pch.h"
)

if(MSVC)
  if(COMET_IS_BUILD_PERFORMANCE_SUMMARY)
    target_compile_options(${COMET_LIBRARY_NAME} PRIVATE "$<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/Bt+>")
  endif()
else()
  if(COMET_IS_BUILD_PERFORMANCE_SUMMARY)
    message(ERROR "${MESSAGE_PREFIX}Unsupported option on current compiler: COMET_IS_BUILD_PERFORMANCE_SUMMARY")
  endif()
endif()

target_compile_definitions(${COMET_LIBRARY_NAME}
  PUBLIC
    COMET_NAME="${COMET_NAME}"
    COMET_VERSION_MAJOR=${COMET_VERSION_MAJOR}
    COMET_VERSION_MINOR=${COMET_VERSION_MINOR}
    COMET_VERSION_PATCH=${COMET_VERSION_PATCH}
    COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
)

# Vcpkg variables ##############################################################
set(VCPKG_BASE_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
set(VCPKG_INCLUDE_DIR "${VCPKG_BASE_DIR}/include")
set(VCPKG_LIBRARY_DIR "${VCPKG_BASE_DIR}/lib")

# Subdirectories ###############################################################
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/editor")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/tests")

# Post-configure operations ####################################################
# Copy configuration file.
message(STATUS
  "${MESSAGE_PREFIX}Copying configuration file to ${CMAKE_CURRENT_BINARY_DIR}...")
if(EXISTS "${PROJECT_SOURCE_DIR}/comet_config.cfg")
  file(
    COPY "${PROJECT_SOURCE_DIR}/comet_config.cfg"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
  )
endif()

# Copy necessary files.
message(STATUS
  "${MESSAGE_PREFIX}Copying assets to ${CMAKE_CURRENT_BINARY_DIR}...")
if(EXISTS "${PROJECT_SOURCE_DIR}/assets")
  file(
    COPY "${PROJECT_SOURCE_DIR}/assets"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
  )
endif()

# Copy "default" resources if necessary (useful for debugging).
if(EXISTS "${PROJECT_SOURCE_DIR}/resources")
  file(
    COPY "${PROJECT_SOURCE_DIR}/resources"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
  )
endif()

# Copy "vk_layer_settings.txt" file if necessary.
if(EXISTS "${PROJECT_SOURCE_DIR}/vk_layer_settings.txt")
  file(
    COPY "${PROJECT_SOURCE_DIR}/vk_layer_settings.txt"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
  )
endif()

message(STATUS "${MESSAGE_PREFIX}Done.")
