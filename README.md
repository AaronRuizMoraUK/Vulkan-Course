# Vulkan-Course

<img src="./Vulkan-logo.png">

Project generated while doing the Udemy course *[Learn the Vulkan API with C++](https://www.udemy.com/course/learn-the-vulkan-api-with-cpp/)* by Ben Cook.

## Build and Run

Requirements
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
- Build and run `main` project

## Controls

Camera can be controlled while pressing the **right mouse button**:

- Use `WSAD` keys to move forward, backwards and sideways.
- Use `QE` keys to move upward and downwards
- Use mouse to rotate and pitch the camera.
- Use mouse wheel to speed up/down camera movement.

## 3rdParty Libraries

- **[glfw](https://github.com/glfw/glfw.git)**: Provides a simple, platform-independent API for creating windows, contexts and surfaces, reading input, handling events, etc.
- **[mathfu](https://github.com/google/mathfu.git)**: Provides a simple and efficient math library with vectors, matrices and quaternions classes.
- **[stb](https://github.com/nothings/stb.git)**: Provides several single-file graphics and audio libraries for C/C++. Used for loading images.
- **[assimp](https://github.com/assimp/assimp.git)**: Library to load various 3D file formats into a shared, in-memory format. Used for loading 3D meshes from FBX and GLTF files.
