# Copyright 2021 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet game object library CMake file
#
################################################################################

# Source files #################################################################
add_library(${GAME_OBJECT_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/game_object/component.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/game_object/game_object.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/game_object/game_object_manager.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/game_object/camera/camera_controls.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/game_object/camera/camera.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/game_object/camera/orthogonal_camera.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/game_object/camera/perspective_camera.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/game_object/model/mesh.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/game_object/model/model.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/game_object/physics/transform.cc"
)

# Compiling ####################################################################
target_include_directories(${GAME_OBJECT_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${GAME_OBJECT_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${GAME_OBJECT_LIBRARY_NAME}
  PUBLIC
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${GAME_OBJECT_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${GAME_OBJECT_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

# Internal libraries.
# Could be better, but for now, a game object could use any part of the engine,
# so we include all the other components.
list(FILTER COMET_LIBRARIES EXCLUDE REGEX "${GAME_OBJECT_LIBRARY_NAME}")

target_link_libraries(${GAME_OBJECT_LIBRARY_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)

