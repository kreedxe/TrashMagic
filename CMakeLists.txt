cmake_minimum_required(VERSION 4.1)
project(TrashMagic)


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


#------------------------| TRASH MAGIC SANDBOX |------------------------#

set(TRASHMAGIC_SRC
    src/main.cpp
)

add_executable(TrashMagic-Sandbox ${TRASHMAGIC_SRC})
target_include_directories(TrashMagic-Sandbox PRIVATE include)
target_link_libraries(TrashMagic-Sandbox PRIVATE glfw glm::glm OpenGL::GL glad)
