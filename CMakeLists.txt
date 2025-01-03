cmake_minimum_required(VERSION 3.5)

project(FilmTool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS 
    Core
    Gui
    Widgets
    Multimedia
    MultimediaWidgets
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Core
    Gui
    Widgets
    Multimedia
    MultimediaWidgets
)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

add_executable(FilmTool
    ${PROJECT_SOURCES}
    mainwindow.h mainwindow.cpp mainwindow.ui
)

target_link_libraries(FilmTool PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)
