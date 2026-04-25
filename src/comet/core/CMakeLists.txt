# Copyright 2026 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet core library CMake file
#
################################################################################

# Packages #####################################################################
find_package(lz4 REQUIRED)

if(WIN32)
  find_path(DETOURS_INCLUDE_DIRS "detours/detours.h" REQUIRED)
  find_library(DETOURS_LIBRARY detours REQUIRED)
endif()

# Source files #################################################################
set(COMET_LIBRARY_SOURCES
  "${PROJECT_SOURCE_DIR}/src/comet/core/compiler.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/compression.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/c_array.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/c_string.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/date.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/debug.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/debug_label.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/define.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/essentials.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/frame_string.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/hash.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/manager.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/os.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/processor.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type_trait.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/version.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/windows.h"

  # Algorithm.
  "${PROJECT_SOURCE_DIR}/src/comet/core/algorithm/back_insert_iterator.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/algorithm/comparator.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/algorithm/inplace_merge.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/algorithm/iterator_utils.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/algorithm/set_difference.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/algorithm/sort.h"

  # Configuration.
  "${PROJECT_SOURCE_DIR}/src/comet/core/conf/configuration_manager.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/conf/configuration_value.cc"

  # Continuation.
  "${PROJECT_SOURCE_DIR}/src/comet/core/continuation/continuation_manager.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/continuation/continuation_utils.h"

  "${PROJECT_SOURCE_DIR}/src/comet/core/continuation/label/continuation_phase_label.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/continuation/type/continuation_phase.h"

  # File system.
  "${PROJECT_SOURCE_DIR}/src/comet/core/file_system/file_system.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/file_system/file_system_label.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/file_system/file_system_type.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/file_system/slash_helper.cc"
  
  # Frame.
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/frame_allocator.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/frame_event.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/frame_manager.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/frame_packet.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/frame_container.h"
  
  # Logic.
  "${PROJECT_SOURCE_DIR}/src/comet/core/logic/game_logic_manager.cc"
  
  # Logger.
  "${PROJECT_SOURCE_DIR}/src/comet/core/logger/logger.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/logger/logger_label.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/logger/logger_type.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/logger/logging.h"
  
  # Concurrency.
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_context.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_label.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_life_cycle.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_primitive.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_utils.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/job.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/job_fence.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/job_label.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/job_utils.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/scheduler.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/worker.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/worker_context.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/thread/thread.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/thread/thread_context.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/provider/thread_provider.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/provider/thread_provider_manager.cc"
  
  # Memory.
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocation_tracking.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/memory.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/memory_label.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/memory_utils.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/tagged_heap.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/virtual_memory.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/allocator.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/free_list_allocator.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/platform_allocator.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/stack_allocator.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/stateful_allocator.cc"
  
  # Type.
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/array.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/bitset.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/buffer_formatter.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/gid.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/handle.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/hash_set.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/iterator.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/map.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/ordered_set.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/primitive.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/region_map.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/ring_queue.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/shared_instance_registry.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/string_id.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/traits.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/tstring.cc"
)

if(NOT (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR
  CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64"))
    message(FATAL_ERROR "Unsupported architecture.")
endif()

if(WIN32)
  list(
    APPEND COMET_LIBRARY_SOURCES
      "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_x86_64_masm_windows.asm"
  )
endif()

if(UNIX)
  list(
    APPEND COMET_LIBRARY_SOURCES
      "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_x86_64_gas_unix.s"
  )
endif()
    
target_sources(${COMET_LIBRARY_NAME}
  PRIVATE
    ${COMET_LIBRARY_SOURCES}
)

# Compiling ####################################################################
target_include_directories(${COMET_LIBRARY_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/"
)

if(WIN32)
  target_include_directories(${COMET_LIBRARY_NAME}
    PRIVATE
      ${DETOURS_INCLUDE_DIRS}
  )
endif()

# Linking ######################################################################
target_link_libraries(${COMET_LIBRARY_NAME}
  PRIVATE
    lz4::lz4
)

if(WIN32)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PUBLIC
      Dbghelp
      $<$<CONFIG:Debug>:${DETOURS_LIBRARY}>
      $<$<CONFIG:DebugRelease>:${DETOURS_LIBRARY}>
  )
endif()
