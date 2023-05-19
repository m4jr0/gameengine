# Copyright 2023 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet math library CMake file
#
################################################################################

# Packages #####################################################################
# OpenGL Mathematics.
find_package(glm REQUIRED)

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/math/bounding_volume.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/math/geometry.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/math/math_commons.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/math/matrix.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/math/plane.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/math/quaternion.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/math/vector.cc"
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${COMET_LIBRARY_NAME}
  PRIVATE
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()
