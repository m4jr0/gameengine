# Copyright 2021 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet rendering library CMake file
#
################################################################################

# Source files #################################################################
add_library(${RENDERING_LIBRARY_NAME}
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/rendering_manager.cpp"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/shader/shader_program.cpp"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/texture/texture_loader.cpp"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/glfw_window.cpp"
    "${PROJECT_SOURCE_DIR}/src/comet/rendering/window/window.cpp"

    "${PROJECT_SOURCE_DIR}/src/comet/rendering/temporary_code.cpp" # To delete.
)

# Compiling ####################################################################
target_include_directories(${RENDERING_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

# Linking ######################################################################
target_link_directories(${RENDERING_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${RENDERING_LIBRARY_NAME}
  PUBLIC
    ${ZLIB_LIBRARY_RELEASE}
    ${Boost_LIBRARIES}
)

if(WIN32)
  target_link_libraries(${RENDERING_LIBRARY_NAME}
    PUBLIC
      assimp::assimp
      bcrypt
      opengl32
      glfw3dll
      glew32
  )
endif()

if(LINUX)
  target_link_libraries(${RENDERING_LIBRARY_NAME}
    PUBLIC
      ${OPENGL_opengl_LIBRARY}
      assimp::assimp
      glfw3
      GLEW
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
