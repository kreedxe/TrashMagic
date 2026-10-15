cmake_minimum_required(VERSION 4.1)
project(TrashMagic)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#------------------------| OPENGL & GLFW & GLM |------------------------#

find_package(OpenGL REQUIRED)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)


#------------------------| GLAD |------------------------#

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)


#------------------------| IMGUI |------------------------#

set(IMGUI_SRC
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})
target_link_libraries(imgui PUBLIC glfw)
target_include_directories(imgui PUBLIC external/imgui)


#------------------------| JOLT PHYSICS |------------------------#

add_subdirectory(external/JoltPhysics/Build)


#------------------------| TRASH MAGIC SANDBOX |------------------------#

set(TRASHMAGIC_SRC
    src/main.cpp
    src/Window.cpp
    src/Renderer.cpp
    src/Shader.cpp
    src/Cube.cpp
    src/Scene.cpp
    src/Transform.cpp
    src/Camera.cpp
    src/LightSource.cpp
    src/Physics.cpp
    src/LightGlowRenderer.cpp
    src/ShadowMapRenderer.cpp
    src/Gui.cpp
    src/Texture.cpp
    src/Cubemap.cpp
    src/SkyboxRenderer.cpp
    src/InputEventSystem.cpp
    src/Sphere.cpp
    src/Pyramid.cpp
)

add_executable(TrashMagic-Sandbox ${TRASHMAGIC_SRC})
target_include_directories(TrashMagic-Sandbox PRIVATE include)
target_compile_definitions(TrashMagic-Sandbox PRIVATE
    TRASHMAGIC_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    TRASHMAGIC_TEXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/textures"
    TRASHMAGIC_FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/misc/fonts/Roboto-Medium.ttf"
)
target_link_libraries(TrashMagic-Sandbox PRIVATE glfw glm::glm OpenGL::GL glad imgui Jolt)
