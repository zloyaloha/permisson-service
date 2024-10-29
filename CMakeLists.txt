cmake_minimum_required(VERSION 3.9)
project(permission-service)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/app/windows/include)
add_subdirectory(src/app)
add_subdirectory(src/server)
