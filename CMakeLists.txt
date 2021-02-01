cmake_minimum_required(VERSION 3.17)
project(box_with_VPL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#   Retrieve conan.cmake
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()

include("${CMAKE_BINARY_DIR}/conan.cmake")

conan_cmake_run(CONANFILE "conanfile.txt" BASIC_SETUP UPDATE BUILD missing)
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
find_package(assimp)
find_package(fmt)
find_package(glfw3)
find_package(glm)

include("${CMAKE_SOURCE_DIR}/src/CMakeLists.txt")
include("${CMAKE_SOURCE_DIR}/include/imgui/CMakeLists.txt")

add_executable(cornell_box
        ${cornell_box_SOURCES})

target_sources(cornell_box PRIVATE
        ${PROJECT_IMGUI_FILES})

#   Creating a resources directory so that binaries can access those
file(MAKE_DIRECTORY
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/")
file(MAKE_DIRECTORY
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/ies")

function(copy_if_different target_executable relative_path)
    add_custom_command(TARGET ${target_executable} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/${relative_path} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${relative_path}")
endfunction()

#   Copy IES files used for testing and executing the application
copy_if_different(cornell_box "resources/ies/TEST.IES")

#   Copy shaders
copy_if_different(cornell_box "resources/shaders/base.vert")
copy_if_different(cornell_box "resources/shaders/base.frag")

#   Copy cornell box scene to resoruces
copy_if_different(cornell_box "resources/cornell_box_multimaterial.obj")
copy_if_different(cornell_box "resources/cornell_box.mtl")

set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE 1)
set(CMAKE_LINK_WHAT_YOU_USE 1)

target_include_directories(cornell_box PRIVATE include)
target_link_libraries(cornell_box PRIVATE assimp::assimp fmt::fmt glfw::glfw glm::glm)

if (CMAKE_CXX_COMPILER_ID MATCHES Clang|AppleClang|GNU)
    target_compile_options(cornell_box PRIVATE -Wall -Wextra -Wpedantic -Wunreachable-code)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    target_compile_options(cornell_box PRIVATE -Wweak-vtables -Wexit-time-destructors -Wglobal-constructors -Wmissing-noreturn)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    target_compile_options(cornell_box PRIVATE /WX /W4 /w44265 /w44061 /w44062 /permissive- /Wv:18 /wd4201)
endif ()