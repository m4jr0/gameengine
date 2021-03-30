add_library(${UTILS_LIBRARY_NAME}
  date.cpp
  file_system.cpp
  logger.cpp
)

target_include_directories(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/comet"
)

target_link_directories(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${Boost_LIBRARIES}
)
