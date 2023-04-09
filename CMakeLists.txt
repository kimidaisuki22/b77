cmake_minimum_required(VERSION 3.21.0)
project(b77 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)
enable_testing()

add_executable(b77 main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


install(TARGETS b77)

find_package(jsoncpp CONFIG REQUIRED )
target_link_libraries(${PROJECT_NAME} PUBLIC  JsonCpp::JsonCpp)