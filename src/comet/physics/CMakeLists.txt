add_library(${PHYSICS_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/physics/physics_manager.cpp"
)

target_include_directories(${PHYSICS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

target_link_directories(${PHYSICS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${PHYSICS_LIBRARY_NAME}
  PUBLIC
    # Put libraries here.
)

if(WIN32)
  target_link_libraries(${PHYSICS_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${PHYSICS_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

target_link_libraries(${PHYSICS_LIBRARY_NAME}
  PRIVATE
    # Put internal libraries here.
)
