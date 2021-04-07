add_library(${EVENT_LIBRARY_NAME}
  "${PROJECT_SOURCE_DIR}/src/comet/event/event.cpp"
  "${PROJECT_SOURCE_DIR}/src/comet/event/event_manager.cpp"
)

target_include_directories(${EVENT_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

target_link_directories(${EVENT_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${EVENT_LIBRARY_NAME}
  PUBLIC
    ${Boost_LIBRARIES}
)

if(WIN32)
  target_link_libraries(${EVENT_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

if(LINUX)
  target_link_libraries(${EVENT_LIBRARY_NAME}
    PUBLIC
      # Put libraries here.
  )
endif()

list(FILTER COMET_LIBRARIES EXCLUDE REGEX "${EVENT_LIBRARY_NAME}")

target_link_libraries(${EVENT_LIBRARY_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)

