# Copyright 2021 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet utils library CMake file
#
################################################################################

# Source files #################################################################
add_library(${UTILS_LIBRARY_NAME}
    "${PROJECT_SOURCE_DIR}/src/comet/utils/date.cpp"
    "${PROJECT_SOURCE_DIR}/src/comet/utils/file_system.cpp"
    "${PROJECT_SOURCE_DIR}/src/comet/utils/logger.cpp"

    "${PROJECT_SOURCE_DIR}/src/comet/utils/structure/exception.hpp"
    "${PROJECT_SOURCE_DIR}/src/comet/utils/structure/ring_queue.hpp"
)

# Compiling ####################################################################
target_include_directories(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${UTILS_LIBRARY_NAME}
  PUBLIC
    # Put libraries here.
)

# Internal libraries.
target_link_libraries(${UTILS_LIBRARY_NAME}
  PRIVATE
    # Put internal libraries here.
)
