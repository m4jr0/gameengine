# Copyright 2022 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet physics library CMake file
#
################################################################################

# Source files #################################################################
add_library(${PHYSICS_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/physics/physics_manager.cc"
)

# Compiling ####################################################################
target_include_directories(${PHYSICS_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${PHYSICS_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${PHYSICS_LIBRARY_NAME}
  PRIVATE
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${PHYSICS_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${PHYSICS_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

# Internal libraries.
target_link_libraries(${PHYSICS_LIBRARY_NAME}
  PRIVATE
    # Put internal libraries here.
)
