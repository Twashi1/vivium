cmake_minimum_required (VERSION 3.12)

project ("vivium4")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(VIVIUM_SOURCES
  "vivium4/core.cpp"
  "vivium4/graphics/primitives/buffer.cpp"
  "vivium4/graphics/primitives/memory_type.cpp"
  "vivium4/graphics/primitives/shader.cpp"
  "vivium4/time/timer.cpp"
  "vivium4/error/log.cpp"
  "vivium4/window.cpp"
  "vivium4/engine.cpp"
  "vivium4/math/math.cpp"
  "vivium4/graphics/primitives/uniform.cpp"
  "vivium4/graphics/primitives/texture.cpp"
  "vivium4/graphics/primitives/descriptor_layout.cpp"
  "vivium4/graphics/primitives/descriptor_set.cpp"
  "vivium4/graphics/resource_manager.cpp"
  "vivium4/graphics/primitives/pipeline.cpp"
  "vivium4/graphics/commands.cpp"
  "vivium4/input.cpp"
  "vivium4/graphics/batch.cpp"
  "vivium4/graphics/gui/font.cpp"
  "vivium4/graphics/gui/visual/text.cpp"
  "vivium4/graphics/gui/base.cpp"
  "vivium4/physics/material.cpp"
  "vivium4/physics/shape.cpp"
  "vivium4/math/aabb.cpp"
  "vivium4/math/polygon.cpp"
  "vivium4/physics/physics.cpp"
  "vivium4/physics/body.cpp"
  "vivium4/math/transform.cpp"
  "vivium4/math/mat2x2.cpp"
  "vivium4/storage.cpp"
  "vivium4/graphics/color.cpp"
  "vivium4/graphics/gui/visual/button.cpp" 
  "vivium4/graphics/primitives/framebuffer.cpp"
  "vivium4/graphics/gui/visual/context.cpp"
  "engine/main.cpp"
  "engine/state.cpp"
  "vivium4/graphics/gui/visual/panel.cpp"
  "vivium4/ecs/defines.cpp"
  "vivium4/ecs/component_array.cpp"
  "vivium4/ecs/registry.cpp"
)
set(VIVIUM_HEADERS
  "vivium4/error/result.h"
  "vivium4/graphics/primitives/buffer.h"
  "vivium4/core.h"
  "vivium4/graphics/primitives/memory_type.h"
  "vivium4/graphics/primitives/shader.h"
  "vivium4/time/timer.h"
  "vivium4/system/os.h"
  "vivium4/window.h"
  "vivium4/math/vec2.h"
  "vivium4/undef.h"
  "vivium4/vivium4.h"
  "vivium4/math/math.h" 
  "vivium4/engine.h"
  "vivium4/storage.h"
  "vivium4/graphics/gui/base.h"
  "vivium4/graphics/primitives/descriptor_set.h"
  "vivium4/graphics/primitives/uniform.h"
  "vivium4/graphics/primitives/texture.h"
  "vivium4/graphics/primitives/descriptor_layout.h"
  "vivium4/graphics/resource_manager.h"
  "vivium4/graphics/commands.h"
  "vivium4/graphics/primitives/pipeline.h"
  "vivium4/input.h"
  "vivium4/graphics/batch.h"
  "vivium4/graphics/gui/visual/text.h"
  "vivium4/graphics/gui/font.h"
  "vivium4/physics/body.h"
  "vivium4/physics/material.h"
  "vivium4/physics/shape.h"
  "vivium4/math/aabb.h"
  "vivium4/math/polygon.h"
  "vivium4/physics/physics.h"
  "vivium4/math/transform.h"
  "vivium4/math/mat2x2.h"
  "vivium4/graphics/gui/visual/button.h"
  "vivium4/graphics/color.h"
  "engine/state.h"
  "vivium4/graphics/primitives/framebuffer.h" 
  "vivium4/graphics/gui/visual/context.h"
  "vivium4/ecs/registry.h"
  "vivium4/serialiser/serialiser.h"
  "vivium4/graphics/gui/visual/panel.h"
  "vivium4/ecs/component_array.h"
  "vivium4/ecs/component_manager.h"
  "vivium4/ecs/defines.h"
  "vivium4/ecs/paged_array.h"
)

add_subdirectory("${CMAKE_SOURCE_DIR}/external/glfw")

add_executable(vivium4 ${VIVIUM_HEADERS} ${VIVIUM_SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET vivium4 PROPERTY CXX_STANDARD 20)
  target_compile_features(vivium4 PUBLIC cxx_std_20)
endif()

target_include_directories(vivium4 PUBLIC "${CMAKE_SOURCE_DIR}/external/freetype/include")
target_include_directories(vivium4 PUBLIC "${CMAKE_SOURCE_DIR}/external/glm")
target_include_directories(vivium4 PUBLIC "${CMAKE_SOURCE_DIR}/external/stb_image")
target_include_directories(vivium4 PUBLIC "${CMAKE_SOURCE_DIR}/external/vulkan/Include")

target_link_libraries(vivium4 PUBLIC
glfw
"${CMAKE_SOURCE_DIR}/external/freetype/static/vs2015-2022/win64/freetype.lib"
"${CMAKE_SOURCE_DIR}/external/vulkan/Lib/vulkan-1.lib"
winmm.lib
)

file(COPY "${CMAKE_SOURCE_DIR}/vivium4/res" DESTINATION "${CMAKE_BINARY_DIR}/vivium4/")
file(COPY "${CMAKE_SOURCE_DIR}/engine/res" DESTINATION "${CMAKE_BINARY_DIR}/engine/")
configure_file("${CMAKE_SOURCE_DIR}/external/vulkan/Bin/glslc.exe" "${CMAKE_BINARY_DIR}/external/vulkan/Bin/glslc.exe" COPYONLY)