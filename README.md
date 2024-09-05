# Vulkan-Course

<img src="./Vulkan-logo.png">

Project generated while doing the Udemy course *[Learn the Vulkan API with C++](https://www.udemy.com/course/learn-the-vulkan-api-with-cpp/)* by Ben Cook.

This branch contains all the code generated up to *Lecture 28: Model Loading* where we can load and render several models with different textures.

Special emphasis has been placed on modularity, separating the Vulkan code into its own library. All the different graphics concepts have been encapsulated in different classes, making easy to read and understand, see [Rendering Engine Architecture](#Rendering-Engine-Architecture) section for more details. Also, additional improvements have been added on top of the course content (see [Vulkan Course Improvements](#Vulkan-Course-Improvements) section).

<img src="./Vulkan_GraphicsEngine.gif">

## Build and Run

Requirements
- Vulkan SDK 1.3. Download installer from https://vulkan.lunarg.com/
- Graphics card with Vulkan support
- Microsoft Windows 
- Microsoft Visual Studio 2022
- CMake 3.28

Steps
- Clone repo
  ````
  git clone https://github.com/AaronRuizMoraUK/Vulkan-Course.git
  ````
- Generate Visual Studio solution using CMake in `build` folder
  ```` 
  cd Vulkan-Course
  mkdir build
  cd build
  cmake .. -G "Visual Studio 17 2022"
  ````
- Open `Vulkan-Course.sln` with Visual Studio
- Build and run `EditorApplication` project

## Controls

Camera can be controlled while pressing the **right mouse button**:

- Use `WSAD` keys to move forward, backwards and sideways.
- Use `QE` keys to move upward and downwards
- Use mouse to rotate and pitch the camera.
- Use mouse wheel to speed up/down camera movement.

## Rendering Engine Architecture

The engine is divided in multiple projects.

| Project | Description |
| ------- | ----------- |
| **Core** | Library with **basic functionality** for the engine, like logging, debugging or the math data structures (vectors, matrices, transform, color, etc.) |
| **Graphics** | Library providing a generic graphics API encapsulating calls to Vulkan API. This is also known as the **Render Hardware Interface (RHI)**. The rest of the engine will communicate with this library and not with Vulkan directly. |
| **Runtime** | This library contains more general constructs to build graphics applications, such as Window, Renderer, Camera or Assets. The renderer has an Scene with objects to render. |
| **EditorApplication** | Project with `main.cpp` that generates the executable. It creates an `Application`, which has a Window, a Renderer and a Camera. `Application` also creates objects and adds them to the renderer. Finally, `Application` also runs the main loop, updating the camera and renders the scene. |
| **Content** | This project contains the Assets folder, the main and 3rdParty CMake files and this *readme* file. |

## Vulkan Course Improvements

I did the following improvements on top of the content provided in the Vulkan course:

#### Project

- Used CMake to generate the project, streamlining the build process and making easier to expand.
- Used a **platform-independent math library** (**[mathfu](https://github.com/google/mathfu.git)**). This prevents the engine from being tied to a specific Graphics API, simplifying the effort to add support for other platforms in the future.
- Organized all 3rdParty libraries under a folder so they appear grouped together in Visual Studio solution.
- Targets setup in CMake with 3 configurations:
  - **Debug**: No optimizations and debug information enabled. Asserts, logs and full debugging available.
  - **Development**: Optimizations and debug information enabled. Asserts, logs and partial debugging available with fast execution.
  - **Release**: Optimizations enabled and no debug information. Asserts, logs and debugging not available. Fastest execution.

#### Application

- Encapsulated GLFW usage into a `Window` class.
- Added a `Camera` class to handle view/projection matrices and camera updates, it doesn't include any Vulkan or Graphics structure. This decouples the camera from 
graphics. The renderer will get the view/projection matrices from the camera and it's the renderer's responsibility to update the uniform buffers for the shaders.
- Supports window minimization.

#### Graphics

- **Encapsulated Vulkan** behind a library (Graphics). This simplifies the addition of other Graphics API in the future.
- Separated Vulkan concepts in different classes. This greatly improves code re-usability and helps understanding Vulkan concepts and the relationships between them.

#### Lighting
 
- Implemented [Blinn-Phong illumination model](https://learnopengl.com/Advanced-Lighting/Advanced-Lighting) in Fragment Shader.
- Implemented [Normal mapping](https://learnopengl.com/Advanced-Lighting/Normal-Mapping) to provide more detailed surfaces to objects.
- Implemented [Directional lighting](https://learnopengl.com/Lighting/Multiple-lights) in Fragment Shader.
- Implemented [Gamma Correction](https://learnopengl.com/Advanced-Lighting/Gamma-Correction) in Fragment Shader for a better quality image due to more accurate lighting calculations in linear space.

#### Assets
 
- Assets only have generic data imported from the files in the Assets folder, they do not include Vulkan or Graphics structures. For example, `TextureAsset` has a buffer of bytes with the image imported from file, but it doesn't include a graphics' `Texture`. Another example is `MeshAsset`, it has the list of positions, indices, etc. imported from FBX or GLTF files, but it doesn't include a graphics' `Buffer`. This keeps the assets system nicely decoupled from graphics structures. Other classes, such as Renderer's `Object`, will use the assets, load their data and then construct their necessary structures from them.
- `MeshAsset` imports all meshes from the 3D file and not just the first one.

## 3rdParty Libraries

- **[glfw](https://github.com/glfw/glfw.git)**: Provides a simple, platform-independent API for creating windows, contexts and surfaces, reading input, handling events, etc.
- **[mathfu](https://github.com/google/mathfu.git)**: Provides a simple and efficient math library with vectors, matrices and quaternions classes.
- **[stb](https://github.com/nothings/stb.git)**: Provides several single-file graphics and audio libraries for C/C++. Used for loading images.
- **[assimp](https://github.com/assimp/assimp.git)**: Library to load various 3D file formats into a shared, in-memory format. Used for loading 3D meshes from FBX and GLTF files.
