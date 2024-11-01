# Copyright 2024 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet resource library CMake file
#
################################################################################

# Source files #################################################################
target_sources(${EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/editor/memory/memory.cc"
)

# Compiling ####################################################################
target_include_directories(${EXECUTABLE_NAME}
  PRIVATE
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${EXECUTABLE_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
    # Put libraries here.
)

target_link_libraries(${EXECUTABLE_NAME}
  PRIVATE
    ${COMET_LIBRARY_NAME}
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()