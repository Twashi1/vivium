# Vivium4

## Build

1. Clone the repository and cd in
> `git clone https://github.com/Twashi1/vivium; cd vivium4`
2. Validate Vulkan version/install
> `python3 scripts/downloadVulkan.py`
3. Create a build directory
> `mkdir build; cd build`
4. Run cmake to build
> `cmake .. -G Ninja`
> `cmake --build .`
5. Run editor
> `./editor` or `editor.exe`

## Dependencies

- Vulkan SDK v1.4+ (installed by script if needed, or see [Vulkan](#vulkan-sdk))
- Lua v5.4+ (installed by CMake)
- GLFW (installed by CMake, but see [GLFW](#glfw) section)
- GLM (packaged with project)
- STB image (packaged with project)

### Vulkan SDK

Follow installation instructions from [the official website](https://vulkan.lunarg.com/sdk/home)

### GLFW

You may have to install the dependencies specified for [compiling GLFW](https://www.glfw.org/docs/latest/compile.html)

