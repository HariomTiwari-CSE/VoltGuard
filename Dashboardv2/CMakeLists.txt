cmake_minimum_required(VERSION 3.16)

project(VoltGuardDashboard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

add_executable(VoltGuardDashboard
    main.cpp
    mainwindow.cpp
    mainwindow.h
    physics_plot_widget.cpp
    physics_plot_widget.h
    traffic_log_model.cpp
    traffic_log_model.h
)

target_link_libraries(VoltGuardDashboard
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
)
