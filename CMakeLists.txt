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
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Generic builds.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Global variables #############################################################
# Executables.
set(EXECUTABLE_NAME "comet_editor")

# Internal library names.
set(COMET_LIBRARY_NAME "comet")

# Other.
set(MESSAGE_PREFIX "[Comet] ")
set(VCPKG_REPO "https://github.com/Microsoft/vcpkg.git")

message(STATUS "${MESSAGE_PREFIX}Build type: ${CMAKE_BUILD_TYPE}")

# OS specifics #################################################################
if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

# Vcpkg setup ##################################################################
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

  set(CMAKE_TOOLCHAIN_FILE
    "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE FILEPATH "Vcpkg toolchain file"
    FORCE
  )

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
  LANGUAGES CXX C ASM
)

if(WIN32)
  enable_language(ASM_MASM)
endif()

# Debug feature groups.
option(COMET_FORCE_DEBUG_DEFINES
  "Define COMET_DEBUG even outside Debug builds"
  OFF
)

option(COMET_FULL_DEBUG_CHECKS
  "Enable every expensive/full debug check used before merging"
  OFF
)

option(COMET_DEBUG_LABELIZE_STRING_IDS
  "Enable string ID labels"
  OFF
)

option(COMET_DEBUG_MEMORY_CHECKS
  "Enable memory-related debug checks"
  OFF
)

option(COMET_DEBUG_TRACK_ALLOCATIONS
  "Enable allocation tracking only"
  OFF
)

option(COMET_DEBUG_LOG_CONTEXT_PREFIX
  "Enable forwarded log context prefixes"
  OFF
)

option(COMET_DEBUG_ENTITY_CHECKS
  "Enable entity debug checks"
  OFF
)

option(COMET_DEBUG_RENDERING_CHECKS
  "Enable rendering debug checks"
  OFF
)

if(CMAKE_CONFIGURATION_TYPES)
  list(APPEND CMAKE_CONFIGURATION_TYPES DebugRelease)
  list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "" FORCE)
endif()

# Multi-config builds.
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
endforeach()

if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUGRELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi" CACHE STRING "" FORCE)
  set(CMAKE_C_FLAGS_DEBUGRELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi" CACHE STRING "" FORCE)
  set(CMAKE_EXE_LINKER_FLAGS_DEBUGRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF" CACHE STRING "" FORCE)
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUGRELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF" CACHE STRING "" FORCE)
endif()

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

option(COMET_ARE_WARNINGS_ERRORS "Treat warnings as errors" ON)
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
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:/permissive->
    $<$<COMPILE_LANGUAGE:CXX>:/Zc:preprocessor>
    $<$<COMPILE_LANGUAGE:CXX>:/W4>
    $<$<COMPILE_LANGUAGE:CXX>:/MP>

    $<$<COMPILE_LANGUAGE:C>:/W4>
    $<$<COMPILE_LANGUAGE:C>:/MP>
  )

  if(COMET_ARE_WARNINGS_ERRORS)
    add_compile_options(
      $<$<COMPILE_LANGUAGE:CXX>:/WX>
      $<$<COMPILE_LANGUAGE:C>:/WX>
    )
  endif()

  if(COMET_IS_ASAN)
    add_compile_options(
      $<$<COMPILE_LANGUAGE:CXX>:/fsanitize=address>
      $<$<COMPILE_LANGUAGE:C>:/fsanitize=address>
    )

    add_link_options(/fsanitize=address)
    add_definitions(-D_DISABLE_STRING_ANNOTATION=1)
    add_definitions(-D_DISABLE_VECTOR_ANNOTATION=1)
  endif()

  if(COMET_IS_TSAN)
    message(FATAL_ERROR "${MESSAGE_PREFIX}Unsupported option on current compiler: COMET_IS_TSAN")
  endif()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -mavx")

  if(COMET_ARE_WARNINGS_ERRORS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic")
  endif()

  if(CMAKE_BUILD_TYPE STREQUAL "DebugRelease")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -DNDEBUG -rdynamic")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g -DNDEBUG")
  endif()

  if(COMET_IS_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
  endif()

  if(COMET_IS_TSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
  endif()
endif()

set(COMET_NAME ${CMAKE_PROJECT_NAME})
string(REPLACE "." ";" VERSION_LIST "${PROJECT_VERSION}")
list(GET VERSION_LIST 0 COMET_VERSION_MAJOR)
list(GET VERSION_LIST 1 COMET_VERSION_MINOR)
list(GET VERSION_LIST 2 COMET_VERSION_PATCH)

add_library(${COMET_LIBRARY_NAME})

target_compile_definitions(${COMET_LIBRARY_NAME}
  PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:DebugRelease>,$<BOOL:${COMET_FORCE_DEBUG_DEFINES}>>:COMET_DEBUG>

    $<$<BOOL:${COMET_FULL_DEBUG_CHECKS}>:COMET_FULL_DEBUG_CHECKS>
    $<$<BOOL:${COMET_DEBUG_LABELIZE_STRING_IDS}>:COMET_DEBUG_LABELIZE_STRING_IDS>
    $<$<BOOL:${COMET_DEBUG_MEMORY_CHECKS}>:COMET_DEBUG_MEMORY_CHECKS>
    $<$<BOOL:${COMET_DEBUG_TRACK_ALLOCATIONS}>:COMET_DEBUG_TRACK_ALLOCATIONS>
    $<$<BOOL:${COMET_DEBUG_LOG_CONTEXT_PREFIX}>:COMET_DEBUG_LOG_CONTEXT_PREFIX>
    $<$<BOOL:${COMET_DEBUG_ENTITY_CHECKS}>:COMET_DEBUG_ENTITY_CHECKS>
    $<$<BOOL:${COMET_DEBUG_RENDERING_CHECKS}>:COMET_DEBUG_RENDERING_CHECKS>

    COMET_NAME="${COMET_NAME}"
    COMET_VERSION_MAJOR=${COMET_VERSION_MAJOR}
    COMET_VERSION_MINOR=${COMET_VERSION_MINOR}
    COMET_VERSION_PATCH=${COMET_VERSION_PATCH}
    COMET_FIBER_EXTERNAL_LIBRARY_SUPPORT
)

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
