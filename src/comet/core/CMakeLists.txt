# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet core library CMake file
#
################################################################################

add_library(${COMET_CORE_TARGET} STATIC)

comet_configure_target(${COMET_CORE_TARGET})

# Packages #####################################################################
find_package(lz4 REQUIRED)

if(WIN32)
  find_path(DETOURS_INCLUDE_DIRS "detours/detours.h" REQUIRED)
  find_library(DETOURS_LIBRARY detours REQUIRED)
endif()

# Source files #################################################################
set(COMET_CORE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/algorithm/back_insert_iterator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/algorithm/comparator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/algorithm/inplace_merge.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/algorithm/iterator_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/algorithm/set_difference.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/algorithm/sort.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/compiler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/compression/compression.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/container/array.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/container/bitset.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/container/c_array.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/container/hash_set.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/container/iterator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/container/map.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/container/ordered_set.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/container/region_map.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/container/ring_queue.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/debug/debug.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/debug/debug_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/define.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/essentials.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/fiber/fiber.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/fiber/fiber_context.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/fiber/fiber_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/fiber/fiber_life_cycle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/fiber/fiber_primitive.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/fiber/fiber_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/file_system/file_system.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/file_system/file_system_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/file_system/file_system_type.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/file_system/slash_helper.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/format/buffer_formatter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/hash/hash.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/job/job.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/job/job_fence.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/job/job_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/job/job_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/job/scheduler.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/job/scheduler_config.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/job/scheduler_pools.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/job/worker.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/job/worker_context.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/logger/logger.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/logger/logger_label.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/logger/logger_type.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/logger/logging.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/bounding_volume.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/geometry.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/math_compression.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/math_interpolation.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/math_scalar.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/matrix.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/numeric_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/plane.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/quaternion.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/math/vector.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/memory/allocator/allocator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/memory/allocator/default_allocator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/memory/allocator/stateful_allocator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/memory/allocator/system_allocator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/memory/memory.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/memory/memory_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/os.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/primitive.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/processor/processor.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/string/c_string.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/string/tstring.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/thread/thread.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/thread/thread_common.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/thread/thread_context.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/thread/thread_provider.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/time/chrono.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/time/date.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/time/time_utils.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/type/gid.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/type/gid_pool.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/type/handle.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/type/handle_pool.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/type/string_id.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/type/string_id_allocator.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/type_trait.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/version.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/comet/core/windows.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/src/compression/compression.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/container/bitset.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/container/region_map.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/debug.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fiber/fiber.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fiber/fiber_context.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fiber/fiber_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fiber/fiber_life_cycle.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fiber/fiber_primitive.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/fiber/fiber_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/file_system/file_system.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/file_system/file_system_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/file_system/slash_helper.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/hash/hash.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/job.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/job_fence.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/job_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/job_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/scheduler.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/scheduler_pools.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/scheduler_queue.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/scheduler_worker_loop.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/worker.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/job/worker_context.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/logger/logger.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/logger/logger_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/math/bounding_volume.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/math/geometry.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/math/math_compression.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/math/plane.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/math/quaternion.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/math/vector.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/allocation_tracking.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/memory_label.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/memory_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/tagged_heap.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/virtual_memory.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/allocator/allocator.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/allocator/default_allocator.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/allocator/stateful_allocator.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/memory/allocator/system_allocator.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/processor/processor.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/string/c_string.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/string/tstring.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/thread/thread.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/thread/thread_context.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/time/chrono.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/time/date.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/time/time_utils.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/type/gid.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/type/gid_pool.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/type/string_id.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/type/string_id_allocator.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cc"
)

if(NOT (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR
        CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64"))
  message(FATAL_ERROR "Unsupported architecture.")
endif()

if(WIN32)
  list(
    APPEND COMET_CORE_SOURCES
      "${CMAKE_CURRENT_SOURCE_DIR}/src/fiber/abi/fiber_x86_64_masm_windows.asm"
  )
endif()

if(UNIX AND NOT APPLE)
  list(
    APPEND COMET_CORE_SOURCES
      "${CMAKE_CURRENT_SOURCE_DIR}/src/fiber/abi/fiber_x86_64_gas_unix.s"
  )
endif()

target_sources(${COMET_CORE_TARGET}
  PRIVATE
    ${COMET_CORE_SOURCES}
)

# Compiling ####################################################################
target_include_directories(${COMET_CORE_TARGET}
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(WIN32)
  target_include_directories(${COMET_CORE_TARGET}
    PRIVATE
      ${DETOURS_INCLUDE_DIRS}
  )
endif()

# Linking ######################################################################
target_link_libraries(${COMET_CORE_TARGET}
  PRIVATE
    lz4::lz4
)

if(WIN32)
  target_link_libraries(${COMET_CORE_TARGET}
    PRIVATE
      Dbghelp
      $<$<CONFIG:Debug>:${DETOURS_LIBRARY}>
      $<$<CONFIG:DebugRelease>:${DETOURS_LIBRARY}>
  )
endif()