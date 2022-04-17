# Copyright 2022 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet input library CMake file
#
################################################################################

# Packages #####################################################################
# OpenGL Mathematics.
find_package(glm REQUIRED)

# Source files #################################################################
add_library(${INPUT_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/input/input_manager.cc"
)

# Compiling ####################################################################
target_include_directories(${INPUT_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${INPUT_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
    glm::glm
)

target_link_libraries(${INPUT_LIBRARY_NAME}
  PRIVATE
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${INPUT_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${INPUT_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

# Internal libraries.
target_link_libraries(${INPUT_LIBRARY_NAME}
  PRIVATE
    # Put internal libraries here.
)
