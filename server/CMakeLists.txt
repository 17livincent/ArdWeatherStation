cmake_minimum_required(VERSION 3.22)

project(WeatherStationServer VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Werror -g)

add_subdirectory(SimpleSocket)
add_subdirectory(src)
