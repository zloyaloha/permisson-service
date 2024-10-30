cmake_minimum_required(VERSION 3.19)
project(Server)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)

set(SERVER_SRC
    main.cpp
    server.cpp
    thread_pool.cpp
)

add_executable(server 
    ${SERVER_SRC}
)

target_link_libraries(server ${Boost_LIBRARIES})