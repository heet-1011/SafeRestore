cmake_minimum_required(VERSION 3.16)
project(SafeRestore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg SvgWidgets)
find_library(BLKID_LIB blkid)

include_directories(include)

add_executable(SafeRestore
    src/main.cpp
    src/Utils.cpp
    src/Fat32Parser.cpp
    src/DriveReader.cpp
    src/MainWindow.cpp
)

target_link_libraries(SafeRestore PRIVATE Qt6::Widgets Qt6::Svg Qt6::SvgWidgets ${BLKID_LIB})