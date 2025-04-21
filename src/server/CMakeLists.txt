cmake_minimum_required(VERSION 3.19)
project(Server)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17 -O)
set(CMAKE_PREFIX_PATH "/usr/local")
set(Sodium_DIR "/usr/lib/x86_64-linux-gnu/cmake/Sodium") # или другой правильный путь


find_package(Boost REQUIRED COMPONENTS system thread)
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(hiredis REQUIRED)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(PQ REQUIRED libpq)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PQXX_INCLUDE_DIRS} ${PQ_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)

set(SERVER_SRC
    main.cpp
    server.cpp
    thread_pool.cpp
)

add_executable(server
    ${SERVER_SRC}
)

target_link_libraries(server ${Boost_LIBRARIES} ${PQXX_LIBRARIES} ${PQ_LIBRARIES} Qt5::Core OpenSSL::SSL OpenSSL::Crypto hiredis redis++)
