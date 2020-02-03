cmake_minimum_required(VERSION 3.1)
project(RenderingKit)

set(CMAKE_CXX_STANDARD 14)

set(RENDERING_KIT_USE_OPENGL_ES_SUBSET OFF CACHE BOOL "Use OpenGL ES-compatible subset")
set(WITH_TINYGLTF ON CACHE BOOL "Enable TinyGLTF loader")

if (RENDERING_KIT_USE_OPENGL_ES_SUBSET OR EMSCRIPTEN)
    set(USING_OPENGL_ES ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if (NOT TARGET zombie_framework)
    add_subdirectory(../framework ${CMAKE_BINARY_DIR}/build-framework)
endif()

# Also add headers so that they're included in generated projects
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/RenderingKit/*.cpp
    ${PROJECT_SOURCE_DIR}/src/RenderingKit/*.hpp
    ${PROJECT_SOURCE_DIR}/src/RenderingKit/gameui/*.cpp

    ${PROJECT_SOURCE_DIR}/include/RenderingKit/gameui/*.hpp
    ${PROJECT_SOURCE_DIR}/include/RenderingKit/utility/*.hpp
)

if (USING_OPENGL_ES)
    list(REMOVE_ITEM sources ${PROJECT_SOURCE_DIR}/src/RenderingKit/DeferredShadingManager.cpp)
endif()

list(APPEND sources
        include/RenderingKit/Model.hpp
        include/RenderingKit/RenderingKit.hpp
        include/RenderingKit/RenderingKitUtility.hpp
        include/RenderingKit/TypeReflection.hpp
        include/RenderingKit/WorldGeometry.hpp
        include/RenderingKit/utility/ShaderGlobal.hpp
        include/RenderingKit/utility/VertexFormat.hpp

        src/RenderingKit/GlobalCache.cpp
        src/RenderingKit/Model.cpp
        src/RenderingKit/wm/WindowManager_SDL2.cpp
        )

if (NOT EMSCRIPTEN)
    list(APPEND sources ${PROJECT_SOURCE_DIR}/dependencies/glew-2.0.0/src/glew.c)
endif()

add_library(${PROJECT_NAME} STATIC ${sources})
add_dependencies(${PROJECT_NAME} zombie_framework)
target_link_libraries(${PROJECT_NAME} PUBLIC zombie_framework)

target_compile_definitions(${PROJECT_NAME} PUBLIC RENDERING_KIT_STATIC=1)
target_compile_definitions(${PROJECT_NAME} PUBLIC STX_NO_STD_VARIANT=1)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC=1)

if (USING_OPENGL_ES)
    target_compile_definitions(${PROJECT_NAME} PUBLIC RENDERING_KIT_USING_OPENGL_ES)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    include
)

if (NOT EMSCRIPTEN)
    target_include_directories(${PROJECT_NAME} PRIVATE dependencies/glew-2.0.0/include)
endif()

# OpenGL
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
elseif(EMSCRIPTEN)
    target_compile_options(${PROJECT_NAME} PRIVATE "SHELL:-s USE_WEBGL2=1")

    # if your CMake is old, you will have to add this to CMAKE_EXE_LINKER_FLAGS in the top-level project
    if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0") 
        target_link_options(${PROJECT_NAME} PUBLIC "SHELL:-s USE_WEBGL2=1")
    endif()
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE GL)
endif()

# SDL2
if (NOT EMSCRIPTEN)
    if (MSVC)
        set(SDL2_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/include)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SDL2_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/lib/x64/SDL2.lib)
            set(SDL2_DLL ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/lib/x64/SDL2.dll)
        else()
            set(SDL2_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/lib/x86/SDL2.lib)
            set(SDL2_DLL ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/lib/x86/SDL2.dll)
        endif()
    endif()

    if (WIN32 AND NOT MSVC)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SDL2_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/x86_64-w64-mingw32/include)
            set(SDL2_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/x86_64-w64-mingw32/lib/libSDL2.dll.a)
            set(SDL2_DLL ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/x86_64-w64-mingw32/bin/SDL2.dll)
        else()
            set(SDL2_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/i686-w64-mingw32/include)
            set(SDL2_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/i686-w64-mingw32/lib/libSDL2.dll.a)
            set(SDL2_DLL ${PROJECT_SOURCE_DIR}/dependencies/SDL2-2.0.5/i686-w64-mingw32/bin/SDL2.dll)
        endif()
    endif()

    if (SDL2_DLL)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL2_DLL}" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)
    endif()

    find_package(SDL2 REQUIRED)

    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${SDL2_INCLUDE_DIR}")
    target_link_libraries(${PROJECT_NAME} PRIVATE "${SDL2_LIBRARY}")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE "SHELL:-s USE_SDL=2")

    # if your CMake is old, you will have to add this to CMAKE_EXE_LINKER_FLAGS in the top-level project
    if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0") 
        target_link_options(${PROJECT_NAME} PUBLIC "SHELL:-s USE_SDL=2")
    endif()
endif()

# tinygltf
if (WITH_TINYGLTF)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TINYGLTF_NOEXCEPTION=1)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TINYGLTF_NO_STB_IMAGE=1)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TINYGLTF_NO_STB_IMAGE_WRITE=1)
    target_include_directories(${PROJECT_NAME} PRIVATE dependencies/tinygltf)
endif()

if((CMAKE_CXX_COMPILER_ID MATCHES Clang) OR (CMAKE_CXX_COMPILER_ID STREQUAL GNU))
    target_compile_options(${PROJECT_NAME} PRIVATE "-Werror=narrowing" "-Werror=switch")
endif()

# Export Assets
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)
