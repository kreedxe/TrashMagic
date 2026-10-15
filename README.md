# TrashMagic

A starting point for a 3D renderer using OpenGL 4.1, GLFW, and GLAD.

`Scene` is a small entity/component registry. Bind user-created `Cube` and
point-light `LightSource` components with a `Transform`, and assign the scene's
`Camera`. Each cube entity references its own shader, so multiple scenes can
reuse different scene data and rendering policies.

`Texture` loads image files with `stb_image`; assign a loaded texture to a cube
with `Cube::setTexture()`, `Sphere::setTexture()`, or `Pyramid::setTexture()`.
The scene can bind cubes, spheres, and pyramids with independent transforms.
The sample scene loads `textures/plank.png` and `textures/prick.png` and applies
them across the different meshes.
`Cubemap` loads a 4-by-3 cross image (six square faces) and can be assigned to a
scene with `Scene::setCubemap()`. The current-scene ImGui panel controls
cubemap brightness, ambient light, and sun direction/color/intensity.

`Renderer::render(scene)` draws a supplied scene using that renderer's own
`RendererSettings`, including up to eight attenuated point lights. Shader files
are stored in `shaders/`; `Shader` handles file loading, compilation, linking,
and uniform updates, while `Cube` owns its vertex data. The camera supports
WASD movement, mouse look, and scroll-wheel FOV adjustment. Press Tab to switch
control focus between the GUI and camera; camera mode hides the cursor, and GUI
mode restores it and enables ImGui keyboard navigation. Escape quits from either
mode. GLFW VSync is enabled
by default, with a 60 FPS frame deadline in the main loop to keep pacing stable
across high-refresh displays. Camera movement uses frame delta time.
Press Escape to close the application.

`Gui` wraps ImGui setup, frame handling, teardown, and a top-corner renderer
debug overlay. `src/main.cpp` demonstrates scene and GUI setup and explicit
destruction; call `Scene::destroy()` before destroying its bound components,
and release OpenGL-backed components and the GUI before the renderer's
window/context. Each scene light is also shown as a small colored cube marker.
