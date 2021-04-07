# Comet editor executable ######################################################
list(APPEND EXECUTABLE_SOURCES
  "${PROJECT_SOURCE_DIR}/src/editor/editor.cpp"
)

add_executable(${EXECUTABLE_NAME} ${EXECUTABLE_SOURCES})

target_precompile_headers(${EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src/comet_precompile.hpp"
    "${PROJECT_SOURCE_DIR}/src/debug_windows.hpp"
)

target_include_directories(${EXECUTABLE_NAME}
  PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    ${VCPKG_INCLUDE_DIR}
)

target_link_directories(${EXECUTABLE_NAME}
  PRIVATE
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${EXECUTABLE_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)
