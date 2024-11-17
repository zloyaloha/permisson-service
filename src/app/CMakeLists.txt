cmake_minimum_required(VERSION 3.9)
project(ClientApp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

include_directories()

set(WINDOWS_SRC
    main.cpp
    command_handler.cpp
    hello_window.cpp
    main_window.cpp
)

qt5_wrap_ui(UI_HEADERS hello_w.ui)
qt5_wrap_ui(UI_HEADERS app_win.ui)

add_executable(app
    ${WINDOWS_SRC}
    ${UI_HEADERS}
)

target_link_libraries(app Qt5::Widgets)
