cmake_minimum_required(VERSION 3.28)

project(mgui-gtk4-examples VERSION 1.0.0 LANGUAGES CXX)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(VersionString)

find_package(Mrc REQUIRED)

write_version_header(${CMAKE_CURRENT_SOURCE_DIR})

add_library(example-rsrc STATIC)
set_target_properties(example-rsrc PROPERTIES LINKER_LANGUAGE CXX)
mrc_target_resources(example-rsrc VERBOSE RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ExampleAppResources/)

add_executable(example-1 ${CMAKE_CURRENT_SOURCE_DIR}/example-1.cpp)
target_link_libraries(example-1 mgui::mgui example-rsrc)

add_executable(example-2 ${CMAKE_CURRENT_SOURCE_DIR}/example-2.cpp)
target_link_libraries(example-2 mgui::mgui example-rsrc)
