# Copyright 2022 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet rendering library CMake file
#
################################################################################

# Packages #####################################################################
# OpenGL Mathematics.
find_package(glm REQUIRED)

# GLFW.
find_package(glfw3 REQUIRED)

# OpenGL.
find_package(glad REQUIRED)
find_package(OpenGL REQUIRED)

# Vulkan.
find_package(Vulkan)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)

# ImGui.
find_package(imgui CONFIG REQUIRED)

# Misc.
find_package(Stb REQUIRED COMPONENTS stb_image)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(assimp REQUIRED)

if(LINUX)
# X11.
  find_package(X11 REQUIRED)
endif()

# Source files #################################################################
add_library(${RENDERING_LIBRARY_NAME}
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/rendering_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/driver.cc"

    # OpenGL.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/opengl_driver.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/shader/shader_program.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/opengl/texture/texture_loader.cc"

    #Vulkan.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_driver.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_initializers.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/driver/vulkan/vulkan_types.h"

    # Windows.
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw_window.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/opengl/opengl_glfw_window.cc"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/window.cc"
)

# Compiling ####################################################################
target_include_directories(${RENDERING_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
    ${STB_INCLUDE_DIRS}
)

if(WIN32)
  target_include_directories(${RENDERING_LIBRARY_NAME}
    PRIVATE
    "${VULKAN_SDK}/Include"
  )
endif()

# Linking ######################################################################
target_link_directories(${RENDERING_LIBRARY_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${RENDERING_LIBRARY_NAME}
  PRIVATE
    ${ZLIB_LIBRARY_RELEASE}
    ${Boost_LIBRARIES}
    glad

  PRIVATE
    assimp::assimp
    tinyobjloader::tinyobjloader
    unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    glm::glm
    imgui::imgui
    glfw
)

if(WIN32)
  target_link_libraries(${RENDERING_LIBRARY_NAME}
    PRIVATE
      "${VULKAN_SDK}/Lib/vulkan-1.lib"
      bcrypt
      opengl32
      glfw3dll
  )
endif()

if(LINUX)
  target_link_libraries(${RENDERING_LIBRARY_NAME}
    PRIVATE
      ${OPENGL_opengl_LIBRARY}
      glfw3
      pthread
      ${CMAKE_DL_LIBS}
      ${X11_X11_LIB}
  )
endif()

# Internal libraries.
# Temporary. For temporary code files.
list(FILTER COMET_LIBRARIES EXCLUDE REGEX "${RENDERING_LIBRARY_NAME}")

target_link_libraries(${RENDERING_LIBRARY_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)

# Should be uncommented when removing temporary code.
# target_link_libraries(${RENDERING_LIBRARY_NAME}
#   PRIVATE
#     # Put internal libraries here.
# )
