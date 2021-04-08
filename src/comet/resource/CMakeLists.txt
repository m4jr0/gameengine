# Copyright 2021 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet resource library CMake file
#
################################################################################

# Source files #################################################################
add_library(${RESOURCE_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/resource/model_resource.cpp"
  "${PROJECT_SOURCE_DIR}/src/comet/resource/resource.cpp"
  "${PROJECT_SOURCE_DIR}/src/comet/resource/resource_manager.cpp"
)

# Compiling ####################################################################
target_include_directories(${RESOURCE_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${RESOURCE_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${RESOURCE_LIBRARY_NAME}
  PUBLIC
    efsw
)

if(WIN32)
  target_link_libraries(${RESOURCE_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${RESOURCE_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

# Internal libraries.
target_link_libraries(${RESOURCE_LIBRARY_NAME}
  PRIVATE
    # Put internal libraries here.
)
