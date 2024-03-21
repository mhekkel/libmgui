cmake_minimum_required(VERSION 3.28)

project(mgui-gtk4-examples VERSION 1.0.0 LANGUAGES CXX)

find_package(Mrc REQUIRED)

add_executable(example-1 ${CMAKE_CURRENT_SOURCE_DIR}/example-1.cpp)
target_link_libraries(example-1 mgui::mgui)
mrc_target_resources(example-1 VERBOSE RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ExampleAppResources/)
