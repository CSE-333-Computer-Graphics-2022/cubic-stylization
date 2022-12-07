cmake_minimum_required(VERSION 3.16)
project(example)
set(CMAKE_VERBOSE_MAKEFILE ON)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Libigl
include(libigl)

# Enable the target igl::glfw
igl_include(glfw)
include_directories(stylization)

add_library(stylization stylization/stylization.cpp)
target_link_libraries(stylization igl::core igl::glfw)


# Add your project files
file(GLOB SRC_FILES *.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC igl::glfw stylization)
