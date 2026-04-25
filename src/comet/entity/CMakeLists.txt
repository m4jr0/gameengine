# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet entity library CMake file
#
################################################################################

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/entity/component.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_changes_fence.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_event.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_memory_context.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/entity_type.cc"
  
    "${PROJECT_SOURCE_DIR}/src/comet/entity/factory/entity_factory_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/factory/handler/entity_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/factory/handler/entity_model_handler.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/factory/handler/entity_primitive_handler.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/entity/type/archetype.h"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/type/entity_flush.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/type/entity_id.h"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/type/pending_entity.h"

    "${PROJECT_SOURCE_DIR}/src/comet/entity/utils/archetype_utils.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/utils/entity_id_utils.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/entity/label/entity_changes_fence_label.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_data_template.h"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_flush.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_lifecycle.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_pending.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_pending_template.h"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_reads_during_flush.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_reads_during_flush_template.h"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_read_iteration_template.h"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_read_operation.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_read_operation_template.h"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_write_operation.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/entity/manager_impl/entity_manager_write_operation_template.h"
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)
