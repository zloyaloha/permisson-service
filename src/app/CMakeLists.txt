cmake_minimum_required(VERSION 3.9)
project(ClientApp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17 -O)

find_package(Boost REQUIRED COMPONENTS system)
find_package(PkgConfig REQUIRED)

pkg_check_modules(HIREDIS REQUIRED hiredis)
pkg_check_modules(REDIS_PP REQUIRED redis++)

include_directories(${HIREDIS_INCLUDE_DIRS})

set(WINDOWS_SRC
    main.cpp
    command_handler.cpp
    hello_window.cpp
    main_window.cpp
)

set(RESOURCES
    resources/resources.qrc
)

qt_add_resources(RESOURCES_GENERATED ${RESOURCES})

qt5_wrap_ui(UI_HEADERS hello_w.ui)
qt5_wrap_ui(UI_HEADERS app_win.ui)

add_executable(app
    ${WINDOWS_SRC}
    ${UI_HEADERS}
    ${RESOURCES_GENERATED}
)
target_sources(app PRIVATE ${WINDOWS_SRC})

target_link_libraries(app Qt5::Widgets ${HIREDIS_LIBRARIES} redis++)
