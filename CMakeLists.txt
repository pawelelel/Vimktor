cmake_minimum_required(VERSION 3.10)

project(Vimktor VERSION 0.5)

set(CMAKE_CXX_STANDARD 23)
file(GLOB SOURCES "*.cpp")

add_executable(Vimktor ${SOURCES})

target_include_directories(Vimktor PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/include"
)

if (WIN32)
    message(STATUS "Windows build")
    add_subdirectory(pcl)
    add_subdirectory(pac)
    target_link_libraries(Vimktor pcl)
    target_link_libraries(Vimktor pac)
    target_compile_definitions(Vimktor PRIVATE PLATFORM_WINDOWS)
elseif (UNIX)
    message(STATUS "Unix build")
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    target_link_libraries(Vimktor ${CURSES_LIBRARIES})
    target_compile_definitions(Vimktor PRIVATE PLATFORM_LINUX)
endif ()