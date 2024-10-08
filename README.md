# Vulkan-Course

<img src="./Vulkan-logo.png">

Project generated while doing the Udemy course *[Learn the Vulkan API with C++](https://www.udemy.com/course/learn-the-vulkan-api-with-cpp/)* by Ben Cook.

Summary of content learned in the course:
- Initiate Vulkan and setup a GPU for usage.
- Validation Layers to validate Vulkan code.
- Setup and use a Swap Chain with triple buffering.
- Setup a Render Pass and Subpasses.
- Create Frame buffers for the render pass.
- Create a Pipeline and Shaders in SPIR-V.
- Create and use a Command Buffer on a Queue.
- Load in Vertex and Index data into Buffers, both in Host Visible and Device Local memory.
- Descriptor Sets and Push Constants.
- Setup a Depth Buffer.
- Implement Textures and manage image layout transitions with pipeline memory barriers.
- Use multiple Subpasses with Input Attachments.

To enhance modularity, I separated the Vulkan code into its own dedicated library. I have also encapsulated various graphics concepts into distinct classes, improving readability and maintainability. For more details, refer to the [Rendering Engine Architecture](#Rendering-Engine-Architecture) section. Additionally, further improvements beyond the course content have been implemented, see [Vulkan Course Improvements](#Vulkan-Course-Improvements) section for more details.

The ['main' branch](https://github.com/AaronRuizMoraUK/Vulkan-Course/tree/main) contains the code generated after finishing the course, where it renders using two subpasses. In the first subpass it draws the 3D world. In the second subpass it draws a full screen triangle and reads from the color and depth images generated in the first subpass.

The ['Lecture21.DynamicUniformBuffers' branch](https://github.com/AaronRuizMoraUK/Vulkan-Course/tree/Lecture21.DynamicUniformBuffers) contains the code generated up to *Lecture 21: Dynamic Uniform Buffers*, where the world matrices of all objects are passed to the vertex shader using a dynamic uniform buffer. In subsequent lectures dynamic uniform buffers are replaced by push constants.

The ['Lecture28.ModelLoading' branch](https://github.com/AaronRuizMoraUK/Vulkan-Course/tree/Lecture28.ModelLoading) contains the code generated up to *Lecture 28: Model Loading*, where we can load and render several models with different textures. This is the last lecture using a render pass with a single subpass. In subsequent lectures a larger refactor is done to use multiple subpasses.

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

## 3rdParty Libraries

- **[glfw](https://github.com/glfw/glfw.git)**: Provides a simple, platform-independent API for creating windows, contexts and surfaces, reading input, handling events, etc.
- **[mathfu](https://github.com/google/mathfu.git)**: Provides a simple and efficient math library with vectors, matrices and quaternions classes.
- **[stb](https://github.com/nothings/stb.git)**: Provides several single-file graphics and audio libraries for C/C++. Used for loading images.
- **[assimp](https://github.com/assimp/assimp.git)**: Library to load various 3D file formats into a shared, in-memory format. Used for loading 3D meshes from FBX and GLTF files.
