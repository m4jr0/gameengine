add_library(${CORE_LIBRARY_NAME}
  engine.cpp
  entry_point.cpp
  manager.cpp

  event/event.cpp
  event/event_manager.cpp

  game_object/component.cpp
  game_object/game_object.cpp
  game_object/game_object_manager.cpp

  game_object/camera/camera_controls.cpp
  game_object/camera/camera.cpp
  game_object/camera/orthogonal_camera.cpp
  game_object/camera/perspective_camera.cpp

  game_object/model/mesh.cpp
  game_object/model/model.cpp

  game_object/physics/transform.cpp

  input/input_manager.cpp

  physics/physics_manager.cpp

  render/render_manager.cpp

  render/shader/shader_program.cpp

  render/texture/texture_loader.cpp
  
  render/window/glfw_window.cpp
  render/window/window.cpp

  resource/model_resource.cpp
  resource/resource.cpp
  resource/resource_manager.cpp

  time/time_manager.cpp

  temporary_code.cpp # To delete.
)

target_include_directories(${CORE_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/comet"
)

target_link_directories(${CORE_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${CORE_LIBRARY_NAME}
  PUBLIC
    efsw
    ${ZLIB_LIBRARY_RELEASE}
    ${Boost_LIBRARIES}
)

if(WIN32)
  target_link_libraries(${CORE_LIBRARY_NAME}
    PUBLIC
      assimp::assimp
      bcrypt
      opengl32
      glfw3dll
      glew32
  )
endif()

if(LINUX)
  target_link_libraries(${CORE_LIBRARY_NAME}
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
