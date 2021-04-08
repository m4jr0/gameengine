# Copyright 2021 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Main CMake file
#
################################################################################

cmake_minimum_required(VERSION 3.18)

# CMake setup ##################################################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# Generic builds.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Multi-config builds.
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

# Global variables #############################################################
# Executables.
set(EXECUTABLE_NAME "comet_editor")
set(TESTS_EXECUTABLE_NAME "comet_tests")

# Internal library names.
set(CORE_LIBRARY_NAME "core")
set(EVENT_LIBRARY_NAME "event")
set(GAME_OBJECT_LIBRARY_NAME "game_object")
set(INPUT_LIBRARY_NAME "input")
set(PHYSICS_LIBRARY_NAME "physics")
set(RENDERING_LIBRARY_NAME "rendering")
set(RESOURCE_LIBRARY_NAME "resource")
set(TIME_LIBRARY_NAME "time")
set(UTILS_LIBRARY_NAME "utils")

# Internal libraries container.
list(APPEND COMET_LIBRARIES
  ${CORE_LIBRARY_NAME}
  ${EVENT_LIBRARY_NAME}
  ${GAME_OBJECT_LIBRARY_NAME}
  ${INPUT_LIBRARY_NAME}
  ${PHYSICS_LIBRARY_NAME}
  ${RENDERING_LIBRARY_NAME}
  ${RESOURCE_LIBRARY_NAME}
  ${TIME_LIBRARY_NAME}
  ${UTILS_LIBRARY_NAME}
)

# Other.
set(MESSAGE_PREFIX "[Comet] ")
set(VCPKG_REPO "https://github.com/Microsoft/vcpkg.git")

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

  if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE
      "${VCPKG_ROOT}\\scripts\\buildsystems\\vcpkg.cmake")
  else()
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  endif()

  message(STATUS "${MESSAGE_PREFIX}CMAKE_TOOLCHAIN_FILE set to"
    " \"${CMAKE_TOOLCHAIN_FILE}\".")
endif()

# Project ######################################################################
message(STATUS
  "${MESSAGE_PREFIX}Building ${CMAKE_PROJECT_NAME} "
  "to ${CMAKE_CURRENT_BINARY_DIR}...")

project("Comet Game Engine"
  VERSION 0.1.0
  DESCRIPTION "Simple video game engine"
  LANGUAGES CXX
)

# Packages #####################################################################
set(Boost_USE_STATIC_LIBS ON)

find_package(assimp REQUIRED)
find_package(Boost REQUIRED COMPONENTS
  filesystem
  serialization
)
find_package(Catch2 REQUIRED)
find_package(efsw REQUIRED)
find_package(glew REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Stb REQUIRED COMPONENTS stb_image)

if(LINUX)
  find_package(X11 REQUIRED)
endif()

# Vcpkg variables ##############################################################
set(VCPKG_BASE_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
set(VCPKG_INCLUDE_DIR "${VCPKG_BASE_DIR}/include")
set(VCPKG_LIBRARY_DIR "${VCPKG_BASE_DIR}/lib")

# Subdirectories ###############################################################
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/editor")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/tests")

# Post-configure operations ####################################################
# Copying necessary files.
message(STATUS
  "${MESSAGE_PREFIX}Copying assets to ${CMAKE_CURRENT_BINARY_DIR}...")
file(COPY "./src/editor/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

if(WIN32) # Workaround for missing DLL (should be fixed in a proper way).
  message(STATUS "${MESSAGE_PREFIX}Copying missing \"glew32.dll\" library...")
  file(
    COPY "${VCPKG_BASE_DIR}/bin/glew32.dll"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
  )
endif()

message(STATUS "${MESSAGE_PREFIX}Done.")
