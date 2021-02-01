include("${CMAKE_CURRENT_LIST_DIR}/events/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/layers/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/rendering/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/scene_management/CMakeLists.txt")

list(TRANSFORM events_SOURCES PREPEND "engine/")
list(TRANSFORM layers_SOURCES PREPEND "engine/")
list(TRANSFORM renderer_SOURCES PREPEND "engine/")
list(TRANSFORM scene_management_SOURCES PREPEND "engine/")

list(APPEND cornell_box_SOURCES ${events_SOURCES})
list(APPEND cornell_box_SOURCES ${layers_SOURCES})
list(APPEND cornell_box_SOURCES ${renderer_SOURCES})
list(APPEND cornell_box_SOURCES ${scene_management_SOURCES})