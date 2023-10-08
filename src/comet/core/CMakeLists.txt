# Copyright 2023 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet core library CMake file
#
################################################################################

# Packages #####################################################################
find_package(lz4 REQUIRED)

# Source files #################################################################
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet/core/c_string.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/compiler.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/compression.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/date.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/debug.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/define.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/file_system.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/generator.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/hash.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/logger.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/os.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/version.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/core/conf/configuration_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/conf/configuration_value.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocation_tracking.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/memory/memory.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/memory/memory_manager.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/frame_allocator.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/stack_allocator.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/string_id_allocator.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/tstring_allocator.cc"

    "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/stl/one_frame_allocator.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/stl/two_frame_allocator.h"

    "${PROJECT_SOURCE_DIR}/src/comet/core/type/exception.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/type/gid.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/type/primitive.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/type/ring_queue.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/type/stl_types.h"
    "${PROJECT_SOURCE_DIR}/src/comet/core/type/string_id.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/core/type/tstring.cc"
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
    "${VCPKG_LIBRARY_DIR}"
)

target_link_libraries(${COMET_LIBRARY_NAME}
  PRIVATE
    lz4::lz4
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
