# Copyright 2022 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet game object library CMake file
#
################################################################################

# Packages #####################################################################
# OpenGL Mathematics.
find_package(glm REQUIRED)

# Source files #################################################################
add_library(${ENTITY_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/entity/entity.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_manager.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_misc_types.cc"
  
  "${PROJECT_SOURCE_DIR}/src/comet/entity/factory/model_entity.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/entity/component/component.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/entity/component/mesh_component.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/entity/component/transform_component.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/entity/component_view.cc"
)

# Compiling ####################################################################
target_include_directories(${ENTITY_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${ENTITY_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
    glm::glm
)

target_link_libraries(${ENTITY_LIBRARY_NAME}
  PRIVATE
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${ENTITY_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${ENTITY_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()

# Internal libraries.
# Could be better, but for now, a game object could use any part of the engine,
# so we include all the other components.
list(FILTER COMET_LIBRARIES EXCLUDE REGEX "${ENTITY_LIBRARY_NAME}")

target_link_libraries(${ENTITY_LIBRARY_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)

