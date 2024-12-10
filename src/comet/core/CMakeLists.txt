# Copyright 2024 m4jr0. All Rights Reserved.
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
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
    find_library(DETOURS_LIBRARY detours REQUIRED)
  endif()
endif()

# Source files #################################################################
set(COMET_LIBRARY_SOURCES
  "${PROJECT_SOURCE_DIR}/src/comet/core/compiler.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/compiler.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/compression.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/c_array.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/c_string.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/date.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/debug.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/define.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/essentials.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/file_system/file_system.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/file_system/slash_helper.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/generator.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/hash.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/logger.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/manager.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/os.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/version.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/conf/configuration_manager.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/conf/configuration_value.cc"
  
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/frame_allocator.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/frame_manager.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/frame_packet.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/stl/one_frame_allocator.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/frame/stl/two_frame_allocator.h"
  
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_context.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_life_cycle.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_primitive.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/fiber/fiber_utils.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/job.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/job_utils.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/scheduler.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/worker.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/job/worker_context.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/thread/thread.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/thread/thread_context.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/provider/thread_provider.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/concurrency/provider/thread_provider_manager.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocation_tracking.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/memory.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/memory_utils.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/tagged_heap.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/virtual_memory.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/allocator.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/platform_allocator.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/free_list_allocator.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/memory/allocator/stack_allocator.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/type/array.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/bitset.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/exception.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/gid.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/hash_set.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/iterator.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/map.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/primitive.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/ring_queue.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/stl_types.h"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/string_id.cc"
  "${PROJECT_SOURCE_DIR}/src/comet/core/type/tstring.cc"

  "${PROJECT_SOURCE_DIR}/src/comet/core/windows.h"
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
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

if(WIN32)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_include_directories(${COMET_LIBRARY_NAME}
      PRIVATE
        ${DETOURS_INCLUDE_DIRS}
    )
  endif()
endif()

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

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${COMET_LIBRARY_NAME}
      PRIVATE
        Dbghelp
        ${DETOURS_LIBRARY}
    )
  endif()
endif()

if(LINUX)
  target_link_libraries(${COMET_LIBRARY_NAME}
    PRIVATE
      # Put libraries here.
  )
endif()
