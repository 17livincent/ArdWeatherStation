cmake_minimum_required(VERSION 3.22)

project(WeatherStationServer VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Werror)

add_subdirectory(SimpleSocket)
add_subdirectory(src)