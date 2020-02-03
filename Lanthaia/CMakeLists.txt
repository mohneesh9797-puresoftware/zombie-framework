cmake_minimum_required(VERSION 3.1)
project(Lanthaia)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 14)
set(ZOMBIE_API_VERSION 202001)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(sources
        src/main.cpp
        src/GameScene.cpp
        src/GameScene.hpp
        src/TitleScene.cpp
        src/TitleScene.hpp
        src/system/LoginSystem.cpp
        src/system/LoginSystem.hpp
        src/system/RenderingSystem.cpp
        src/system/RenderingSystem.hpp
)

add_subdirectory(../framework ${CMAKE_BINARY_DIR}/build-framework)
add_subdirectory(../RenderingKit ${CMAKE_BINARY_DIR}/build-RenderingKit)

add_executable(${PROJECT_NAME} ${sources})

add_dependencies(${PROJECT_NAME} zombie_framework)
target_link_libraries(${PROJECT_NAME} zombie_framework)

add_dependencies(${PROJECT_NAME} RenderingKit)
target_link_libraries(${PROJECT_NAME} RenderingKit)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

if (EMSCRIPTEN AND ${CMAKE_VERSION} VERSION_LESS "3.13.0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s FORCE_FILESYSTEM=1 -s USE_FREETYPE=1 -s USE_SDL=2 -s USE_WEBGL2=1")
endif()

# Copy assets
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" $<TARGET_FILE_DIR:${PROJECT_NAME}>)
