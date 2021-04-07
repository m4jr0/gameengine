add_library(${UTILS_LIBRARY_NAME}
    "${PROJECT_SOURCE_DIR}/src/comet/utils/date.cpp"
    "${PROJECT_SOURCE_DIR}/src/comet/utils/file_system.cpp"
    "${PROJECT_SOURCE_DIR}/src/comet/utils/logger.cpp"

    "${PROJECT_SOURCE_DIR}/src/comet/utils/structure/exception.hpp"
    "${PROJECT_SOURCE_DIR}/src/comet/utils/structure/ring_queue.hpp"
)

target_include_directories(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/src/"
)

target_link_directories(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${VCPKG_LIBRARY_DIR}
)

target_link_libraries(${UTILS_LIBRARY_NAME}
  PUBLIC
    ${Boost_LIBRARIES}
)

list(FILTER COMET_LIBRARIES EXCLUDE REGEX "${UTILS_LIBRARY_NAME}")

target_link_libraries(${UTILS_LIBRARY_NAME}
  PRIVATE
    ${COMET_LIBRARIES}
)