cmake_minimum_required(VERSION 3.10)

project(LinuxiDeviceTools)
add_executable(LinuxiDeviceTools main.cpp)
find_library(imob imobiledevice)
find_library(irec irecovery)
find_library(plist plist)
target_link_libraries(LinuxiDeviceTools "${imob}" "${irec}" "${plist}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)