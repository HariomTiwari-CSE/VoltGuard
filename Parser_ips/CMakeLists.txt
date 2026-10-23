cmake_minimum_required(VERSION 3.16)
project(VoltGuardDashboard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Point CMake directly to system prefix where Qt6 modules reside

list(APPEND CMAKE_PREFIX_PATH "/usr" "/usr/lib/cmake" "/usr/lib/cmake/Qt6")

find_package(Qt6Widgets REQUIRED)
find_package(Qt6Charts REQUIRED)

add_executable(VoltGuardUI
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

target_link_libraries(VoltGuardUI PRIVATE Qt6::Widgets Qt6::Charts)
