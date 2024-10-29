cmake_minimum_required(VERSION 3.19)
project(Server)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Boost REQUIRED COMPONENTS system)
set(CMAKE_CXX_STANDARD 17)

set(SERVER_SRC
    main.cpp
    server.cpp
)

add_executable(server 
    ${SERVER_SRC}
)