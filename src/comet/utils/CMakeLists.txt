add_library(${UTILS_LIBRARY_NAME}
  date.cpp
  file_system.cpp
  logger.cpp

  structure/exception.hpp
  structure/ring_queue.hpp
)

target_include_directories(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/comet"
)

target_link_directories(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${Boost_LIBRARIES}
)
