# Copyright 2023 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet game object library CMake file
#
################################################################################

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/entity/archetype.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/component.h"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_component.h"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_id.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_type.cc"
  
    "${PROJECT_SOURCE_DIR}/src/comet/entity/factory/entity_factory_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/factory/handler/entity_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/factory/handler/entity_model_handler.cc"
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
