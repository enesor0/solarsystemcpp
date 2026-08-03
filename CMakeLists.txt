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

find_path(STB_INCLUDE_DIR
    NAMES stb_image.h
    PATHS ${CMAKE_PREFIX_PATH}
    PATH_SUFFIXES include
    REQUIRED
)

add_executable(SolarSystem
    source/main.cpp
    source/core/simulation_clock.cpp
	source/graphics/bloom.cpp
    source/graphics/shader.cpp
	source/graphics/mesh.cpp
    source/graphics/camera.cpp
	source/graphics/asteroid_belt.cpp
	source/graphics/space_station_mesh.cpp
    source/graphics/hud.cpp
    source/graphics/orbit_path.cpp
	 source/graphics/orbit_trail.cpp
	 source/graphics/shadow_map.cpp
	source/graphics/ring.cpp
	source/graphics/starfield.cpp
	source/graphics/sphere.cpp
	source/graphics/texture.cpp
	source/graphics/texture_library.cpp
    source/world/planet.cpp
    source/world/planet_facts.cpp
	source/world/moon.cpp
	source/world/space_station.cpp
	source/world/solar_system.cpp
)

target_include_directories(SolarSystem PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
    ${STB_INCLUDE_DIR}
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

add_custom_command(TARGET SolarSystem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:SolarSystem>/assets"
)
