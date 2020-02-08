cmake_minimum_required(VERSION 3.1)
project(GfxObsession)

set(CMAKE_CXX_STANDARD 17)

add_library(GfxObsession
		include/Gfx/RenderingSystem.hpp

		src/RenderingSystem.cpp
		)

target_include_directories(${PROJECT_NAME} PUBLIC include)

add_dependencies(GfxObsession RenderingKit zombie_framework)
target_link_libraries(GfxObsession RenderingKit zombie_framework)
