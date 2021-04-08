add_library(${RESOURCE_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/resource/model_resource.cpp"
  "${PROJECT_SOURCE_DIR}/src/comet/resource/resource.cpp"
  "${PROJECT_SOURCE_DIR}/src/comet/resource/resource_manager.cpp"
)

target_include_directories(${RESOURCE_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

target_link_directories(${RESOURCE_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${RESOURCE_LIBRARY_NAME}
  PUBLIC
    efsw
)

if(WIN32)
  target_link_libraries(${RESOURCE_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${RESOURCE_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

target_link_libraries(${RESOURCE_LIBRARY_NAME}
  PRIVATE
    # Put internal libraries here.
)
