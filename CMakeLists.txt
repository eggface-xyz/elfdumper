cmake_minimum_required(VERSION 3.22)
project (elfdumper)

add_library(standard_project_options INTERFACE)
target_compile_features(standard_project_options INTERFACE cxx_std_20)
target_compile_options(standard_project_options INTERFACE -fno-pie)

add_library(elf STATIC util.cpp)
target_include_directories(elf PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(elf PUBLIC standard_project_options)

add_executable(elfdumper main.cpp)
target_link_libraries(elfdumper elf fmt)
target_link_options(elfdumper PRIVATE -no-pie)
