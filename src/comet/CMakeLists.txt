# Copyright 2021 m4jr0. All Rights Reserved.
# Use of this source code is governed by the MIT
# license that can be found in the LICENSE file.

################################################################################
#
# Comet engine CMake file
#
################################################################################

# Subdirectories ###############################################################
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/core")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/event")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/game_object")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/input")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/physics")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/rendering")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/resource")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/time")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/comet/utils")
