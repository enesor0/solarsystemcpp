cmake_minimum_required(VERSION 3.20)

project(SolarSystemCpp LANGUAGES CXX)

list(PREPEND CMAKE_PREFIX_PATH
    "C:/Users/eneso/source/library/vcpkg/installed/x64-windows"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(SolarSystem
    source/main.cpp
    source/graphics/Shader.cpp
    source/graphics/Mesh.cpp
)

target_include_directories(SolarSystem PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
)

target_link_libraries(SolarSystem PRIVATE
    glfw
    glad::glad
    OpenGL::GL
    glm::glm
)

add_custom_command(TARGET SolarSystem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:SolarSystem>/shaders"
)

add_custom_command(TARGET SolarSystem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "C:/Users/eneso/source/library/vcpkg/installed/x64-windows/debug/bin/glfw3.dll"
    "$<TARGET_FILE_DIR:SolarSystem>"
)