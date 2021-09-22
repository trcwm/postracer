
cmake_minimum_required(VERSION 3.12)
project(curvetracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets SerialPort REQUIRED)
find_package(Threads)

set(SRC 
    src/graph.cpp
    src/serialctrl.cpp
    src/mainwindow.cpp
    src/main.cpp)

add_executable(curvetracer ${SRC})
target_link_libraries(curvetracer Qt5::Widgets Qt5::SerialPort Threads::Threads)

