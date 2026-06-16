# Set the minimum CMake version
cmake_minimum_required(VERSION 3.10)

project(00_DoF)

add_executable(00_DoF
	main.cpp
	renderer.hpp
	scene_definition.hpp
)
target_sources(00_DoF PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/src/glad.c
)
target_link_libraries(00_DoF utils glm::glm glfw OpenGL::GL)
target_include_directories(00_DoF PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/include
	${CMAKE_CURRENT_SOURCE_DIR}/../utils
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
)
