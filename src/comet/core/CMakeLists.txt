# Copyright 2021 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet core library CMake file
#
################################################################################

# Source files #################################################################
add_library(${CORE_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/core/engine.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/entry_point.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/id/string_id.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/logger.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/manager.cc"
)

# Compiling ####################################################################
target_include_directories(${CORE_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${CORE_LIBRARY_NAME}
  PUBLIC
    "${VCPKG_LIBRARY_DIR}"
)

target_link_libraries(${CORE_LIBRARY_NAME}
  PUBLIC
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${CORE_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${CORE_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

# Internal libraries.
# Could be better, but for now, the engine could use any part of the engine, so
# we include all the other components.
list(FILTER COMET_LIBRARIES EXCLUDE REGEX "${CORE_LIBRARY_NAME}")

target_link_libraries(${CORE_LIBRARY_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)
