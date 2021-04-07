add_library(${INPUT_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/input/input_manager.cpp"
)

target_include_directories(${INPUT_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

target_link_directories(${INPUT_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${INPUT_LIBRARY_NAME}
  PUBLIC
    ${Boost_LIBRARIES}
)

if(WIN32)
  target_link_libraries(${INPUT_LIBRARY_NAME}
    PUBLIC
      opengl32
      glfw3dll
      glew32
  )
endif()

if(LINUX)
  target_link_libraries(${INPUT_LIBRARY_NAME}
    PUBLIC
      ${OPENGL_opengl_LIBRARY}
      glfw3
      GLEW
  )
endif()

list(FILTER COMET_LIBRARIES EXCLUDE REGEX "${INPUT_LIBRARY_NAME}")

target_link_libraries(${INPUT_LIBRARY_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)
