# TrashMagic

A starting point for a 3D renderer using OpenGL 4.1, GLFW, and GLAD.

## Architecture

`Scene` is the ECS registry and owns scene-level data (camera, lighting, and
physics settings). Application-bound meshes and lights remain externally
owned; point lights created in the editor are owned by the scene and released
when removed or when the scene is destroyed. `Physics` consumes a scene and
updates physics-backed entity transforms. `Renderer` consumes the same scene
and renders it without owning or mutating scene entities. `Gui` provides the
ImGui-backed editor for creating and removing entities and editing their
component values.

## Editor

The GLFW window opens maximized within the available desktop area, not in
exclusive fullscreen. The editor uses a Roboto UI font and a custom dark
slate/blue theme. Its title-less hierarchy and inspector are styled as flat
side panels rather than floating windows, alongside a top menu bar and bottom status bar. Select the
scene root to edit gravity and scene lighting; select an entity to edit its
name, transform, light settings (including point-light range), and available
physics body values. Use the Create menu to add cubes, spheres, pyramids, or
point lights; select an entity and choose Delete Entity in the inspector to
remove it. The View menu opens the hierarchy, inspector, and renderer
configuration window. That window controls solid, wireframe, and point draw
modes; perspective or orthographic projection; camera field of view, view
height, and clip planes; clear color; and rendering diagnostics such as depth
testing, shadows, skybox, light markers, and light glow.

`Scene` is a small entity/component registry. Bind user-created mesh and
point-light `LightSource` components with a `Transform`, and assign the scene's
`Camera`. Each mesh entity references its own shader, so multiple scenes can
reuse different scene data and rendering policies.

`Physics` simulates scene entities with optional `PhysicsBody` components,
using Jolt for static, dynamic, and kinematic rigid bodies, box and sphere
colliders, per-scene gravity, and solid or sensor contacts. Call
`physics.step(scene, deltaSeconds)` each frame before rendering. Bodies without
a physics component remain unchanged by the simulation. Remove a scene from the
physics engine with `physics.removeScene(scene)` before destroying that scene.

`Texture` loads image files with `stb_image`; assign a loaded texture to a cube
with `Cube::setTexture()`, `Sphere::setTexture()`, or `Pyramid::setTexture()`.
The scene can bind cubes, spheres, and pyramids with independent transforms.
Cube textures repeat according to the cube's dimensions instead of stretching
across scaled surfaces.
The sample scene loads `textures/plank.png` and `textures/prick.png` and applies
them across the different meshes.
`Cubemap` loads a 4-by-3 cross image (six square faces) and can be assigned to a
scene with `Scene::setCubemap()`. The scene inspector controls cubemap
brightness, ambient light, sun direction/color/intensity, roughness, specular
strength, and gravity.

`Renderer::render(scene)` draws a supplied scene using that renderer's own
`RendererSettings`, including solid, wireframe, and point modes; configurable
debug rendering; up to eight radius-limited point lights; and a directional sun
shadow map with filtered edges. The lighting currently uses linear-space,
roughness-controlled Blinn-Phong highlights and diffuse response; this is an
intentional stepping stone, not a full PBR implementation. Shader files are
stored in `shaders/`; `Shader` handles file loading, compilation, linking, and
uniform updates, while `Cube` owns its vertex data. The camera supports
WASD movement, mouse look, and scroll-wheel FOV adjustment. Press Tab to switch
control focus between the GUI and camera; camera mode hides the cursor, and GUI
mode restores it and enables ImGui keyboard navigation. Escape quits from either
mode. GLFW VSync is enabled
by default, with a 60 FPS frame deadline in the main loop to keep pacing stable
across high-refresh displays. Camera movement uses frame delta time.
Press Escape to close the application.

`Gui` wraps ImGui setup, frame handling, editor panels, and teardown.
`src/main.cpp` demonstrates scene and GUI setup and explicit
destruction; call `Scene::destroy()` before destroying externally bound
components, and release OpenGL-backed components and the GUI before the
renderer's window/context. Each scene light is also shown as a small colored
cube marker with a soft glow scaled by its range.
