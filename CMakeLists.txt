cmake_minimum_required(VERSION 3.15)
project(cpp-log)
set(target log)
set(src src/api.cpp src/impl.cpp)
add_library(${target} STATIC ${src})
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
