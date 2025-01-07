cmake_minimum_required(VERSION 3.5)

# 定义版本号
set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 2)
set(APP_VERSION_PATCH 0)

# 创建版本号字符串
set(APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}")

project(FilmTool VERSION ${APP_VERSION} LANGUAGES CXX)

# 配置版本头文件
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
    mainwindow.ui
    customlistwidget.h
    customlistwidget.cpp
    splashwindow.h
    splashwindow.cpp
    resources.qrc
)

add_executable(FilmTool
    ${PROJECT_SOURCES}
)

target_link_libraries(FilmTool PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)

# 复制 FFmpeg 到输出目录
if(WIN32)
    set(FFMPEG_FILES
        ${CMAKE_SOURCE_DIR}/third_party/ffmpeg/ffmpeg.exe
        ${CMAKE_SOURCE_DIR}/third_party/ffmpeg/ffplay.exe
        ${CMAKE_SOURCE_DIR}/third_party/ffmpeg/ffprobe.exe
    )
    
    foreach(FFMPEG_FILE ${FFMPEG_FILES})
        add_custom_command(TARGET FilmTool POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_FILE}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
    endforeach()
endif()

# 添加包含路径
target_include_directories(FilmTool PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)
