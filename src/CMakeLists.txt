set(cornell_box_SOURCES main.cpp 
                        glad.c)

include("${CMAKE_CURRENT_LIST_DIR}/engine/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/ies/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/utility/CMakeLists.txt")

list(TRANSFORM cornell_box_SOURCES PREPEND "src/")
