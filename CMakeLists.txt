cmake_minimum_required (VERSION 3.12)

project ("vivium4")

# TODO: use find_package for all?

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

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
  "vivium4/ecs/group.cpp"
  "vivium4/graphics/gui/visual/container.cpp"
  "vivium4/graphics/gui/visual/slider.cpp"
 "vivium4/graphics/gui/visual/sprite.cpp"
 "vivium4/math/atlas.cpp"
 "vivium4/graphics/texture_format.cpp"
 "vivium4/graphics/image_load.cpp"
 "engine/tree_container.cpp"
 "vivium4/graphics/gui/visual/debugrect.cpp"
 "vivium4/graphics/gui/visual/entry.cpp" "engine/components.cpp" "vivium4/serialiser/serialiser.cpp" "runtime/main.cpp" "runtime/runtime.cpp" "vivium4/ecs/component_manager.cpp")
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
  "vivium4/ecs/group.h" 
  "engine/ecstest.h"
  "vivium4/graphics/gui/visual/container.h"
  "vivium4/graphics/gui/visual/slider.h"
"vivium4/graphics/gui/visual/sprite.h"
"vivium4/math/atlas.h"
"vivium4/graphics/texture_format.h"
"vivium4/graphics/image_load.h"
  "engine/tree_container.h"
  "engine/engine.h"
  "vivium4/graphics/gui/visual/debugrect.h"
  "vivium4/graphics/gui/visual/entry.h"
  "vivium4/input_string.h"
 "engine/components.h"  "runtime/runtime.h" "vivium4/ecs/type_info.h" "vivium4/ecs/group_impl.h"  "vivium4/ecs/registry_impl.h"  "vivium4/ecs/component_manager_impl.h")

add_subdirectory("${CMAKE_SOURCE_DIR}/external/glfw")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/linux/freetype-2.13.3")

add_executable(v4 ${VIVIUM_HEADERS} ${VIVIUM_SOURCES})

set_property(TARGET v4 PROPERTY CXX_STANDARD 20)
target_compile_features(v4 PUBLIC cxx_std_20)

string(LENGTH "${CMAKE_SOURCE_DIR}/" VIVIUM_SOURCE_PATH_SIZE)
add_definitions("-DVIVIUM_SOURCE_PATH_SIZE=${VIVIUM_SOURCE_PATH_SIZE}")

# Header-only, build same regardless of system
target_include_directories(v4 PUBLIC "${CMAKE_SOURCE_DIR}/external/glm")
target_include_directories(v4 PUBLIC "${CMAKE_SOURCE_DIR}/external/stb_image")

if (WIN32)
  target_include_directories(v4 PUBLIC "${CMAKE_SOURCE_DIR}/external/win/freetype/include")
  target_include_directories(v4 PUBLIC "${CMAKE_SOURCE_DIR}/external/win/vulkan/Include")
  target_include_directories(v4 PUBLIC "${CMAKE_SOURCE_DIR}/external/win/lua/include")

  target_link_libraries(v4 PUBLIC
    glfw
    "${CMAKE_SOURCE_DIR}/external/win/freetype/static/vs2015-2022/win64/freetype.lib"
    "${CMAKE_SOURCE_DIR}/external/win/vulkan/Lib/vulkan-1.lib"
    "${CMAKE_SOURCE_DIR}/external/win/lua/lua54.lib"
    winmm.lib
  )
else()
  #target_include_directories(v4 PUBLIC "${CMAKE_SOURCE_DIR}/external/linux/freetype/include/freetype2")
  #target_include_directories(v4 PRIVATE ${FREETYPE_INCLUDE_DIRS})
  target_include_directories(v4 PUBLIC "${CMAKE_SOURCE_DIR}/external/linux/vulkan/x86_64/include")
  target_include_directories(v4 PUBLIC "${CMAKE_SOURCE_DIR}/external/linux/lua/include")

  target_link_libraries(v4 PUBLIC
    glfw
    freetype
    #${FREETYPE_LIBRARIES}
  #  "${CMAKE_SOURCE_DIR}/external/linux/freetype/lib/libfreetype.a"
    "${CMAKE_SOURCE_DIR}/external/linux/vulkan/x86_64/lib/libvulkan.so.1.4.321"
    "${CMAKE_SOURCE_DIR}/external/linux/lua/liblua54.a"
  )
endif()

file(COPY "${CMAKE_SOURCE_DIR}/vivium4/res" DESTINATION "${CMAKE_BINARY_DIR}/vivium4/")
file(COPY "${CMAKE_SOURCE_DIR}/engine/res" DESTINATION "${CMAKE_BINARY_DIR}/engine/")

if (WIN32)
  configure_file("${CMAKE_SOURCE_DIR}/external/win/vulkan/Bin/glslc.exe" "${CMAKE_BINARY_DIR}/external/win/vulkan/Bin/glslc.exe" COPYONLY)
else()
  configure_file("${CMAKE_SOURCE_DIR}/external/linux/vulkan/x86_64/bin/glslc" "${CMAKE_BINARY_DIR}/external/linux/vulkan/x86_64/bin/glslc" COPYONLY)
endif()
