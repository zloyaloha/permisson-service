cmake_minimum_required(VERSION 3.9)
project(permission-service)

set(CMAKE_CXX_STANDARD 17)
find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)


include_directories(src/common)
add_subdirectory(src/app)
add_subdirectory(src/server)
