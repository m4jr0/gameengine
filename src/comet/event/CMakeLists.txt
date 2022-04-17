# Copyright 2022 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet event library CMake file
#
################################################################################

# Source files #################################################################
add_library(${EVENT_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/event/event.h"
  "${PROJECT_SOURCE_DIR}/src/comet/event/event_manager.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/event/window_event.cc"
)

# Compiling ####################################################################
target_include_directories(${EVENT_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${EVENT_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${EVENT_LIBRARY_NAME}
  PUBLIC
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${EVENT_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${EVENT_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

# Internal libraries.
target_link_libraries(${EVENT_LIBRARY_NAME}
  PRIVATE
    # Put internal libraries here.
)
