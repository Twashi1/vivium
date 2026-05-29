cmake_minimum_required (VERSION 3.24)

project ("vivium4" LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
  "vivium4/graphics/gui/visual/debugrect.cpp"
  "vivium4/graphics/gui/visual/entry.cpp"
  "vivium4/serialiser/serialiser.cpp"
  "vivium4/ecs/component_manager.cpp"
  "vivium4/lang/guif/ast.cpp"
  "vivium4/lang/guif/interpreter.cpp"
  "vivium4/lang/guif/lexer.cpp"
  "vivium4/lang/guif/parser.cpp"
  "vivium4/test/suite.cpp"
  "vivium4/math/random.cpp"
)

set(VIVIUM_HEADERS
  "vivium4/math/random.h"
  "vivium4/test/suite.h"
  "vivium4/test/suite_impl.h"
  "vivium4/error/result.h"
  "vivium4/utility/type_names.h"
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
  "vivium4/graphics/gui/visual/container.h"
  "vivium4/graphics/gui/visual/slider.h"
  "vivium4/graphics/gui/visual/sprite.h"
  "vivium4/math/atlas.h"
  "vivium4/graphics/texture_format.h"
  "vivium4/graphics/image_load.h"
  "vivium4/graphics/gui/visual/debugrect.h"
  "vivium4/graphics/gui/visual/entry.h"
  "vivium4/input_string.h"
  "vivium4/ecs/type_info.h"
  "vivium4/ecs/group_impl.h"
  "vivium4/ecs/registry_impl.h"
  "vivium4/ecs/component_manager_impl.h"
  "vivium4/lang/guif/ast.h"
  "vivium4/lang/guif/interpreter.h"
  "vivium4/lang/guif/interpreter_impl.h"
  "vivium4/lang/guif/lexer.h"
  "vivium4/lang/guif/parser.h"
)

set(EDITOR_HEADERS
  "engine/components.h"
  "engine/ecstest.h"
  "engine/engine.h"
  "engine/state.h"
  "engine/tree_container.h"
)

set(EDITOR_SOURCES
  "engine/components.cpp"
  "engine/state.cpp"
  "engine/tree_container.cpp"
)

set(RUNTIME_HEADERS
  "runtime/runtime.h"
)

set(RUNTIME_SOURCES
  "runtime/runtime.cpp"
)

set(TESTING_HEADERS
  "testing/ecs.h"
  "testing/shared.h"
)

set(TESTING_SOURCES
  "testing/ecs.cpp"
)

set(VERLET_HEADERS "verlet/state.h")
set(VERLET_SOURCES "verlet/state.cpp")

find_package(Vulkan REQUIRED)
find_package(VulkanUtilityLibraries CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(lua CONFIG REQUIRED)

add_library(vivium_core STATIC ${VIVIUM_SOURCES} ${VIVIUM_HEADERS})
target_include_directories(vivium_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(vivium_core PUBLIC Vulkan::Vulkan Vulkan::UtilityHeaders glfw glm::glm Freetype::Freetype unofficial::lua::lua)

add_executable(runtime ${RUNTIME_HEADERS} ${RUNTIME_SOURCES} "runtime/main.cpp")
add_executable(editor ${EDITOR_HEADERS} ${EDITOR_SOURCES} "engine/main.cpp")
add_executable(test ${TESTING_HEADERS} ${TESTING_SOURCES} "testing/main.cpp")
add_executable(verlet_run ${VERLET_HEADERS} ${VERLET_SOURCES} "verlet/main.cpp")

target_link_libraries(runtime PRIVATE vivium_core)
target_link_libraries(editor PRIVATE vivium_core)
target_link_libraries(test PRIVATE vivium_core)
target_link_libraries(verlet_run PRIVATE vivium_core)

string(LENGTH "${CMAKE_SOURCE_DIR}/" VIVIUM_SOURCE_PATH_SIZE)
message(VIVIUM_SOURCE_PATH_SIZE="${VIVIUM_SOURCE_PATH_SIZE}")
add_definitions("-DVIVIUM_SOURCE_PATH_SIZE=${VIVIUM_SOURCE_PATH_SIZE}")

# TODO: implement these with custom target
file(COPY "${CMAKE_SOURCE_DIR}/vivium4/res" DESTINATION "${CMAKE_BINARY_DIR}/vivium4/")
file(COPY "${CMAKE_SOURCE_DIR}/engine/res" DESTINATION "${CMAKE_BINARY_DIR}/engine/")
file(COPY "${CMAKE_SOURCE_DIR}/verlet/res" DESTINATION "${CMAKE_BINARY_DIR}/verlet/")

# TODO: need to see how this works with vcpkg
# TODO: need to redo-this and find the glslc binaries from the correct location
# Probably need to redo this
# Vulkan_GLSLC_EXECUTABLE
if (WIN32)
  configure_file("${CMAKE_SOURCE_DIR}/external/win/vulkan/Bin/glslc.exe" "${CMAKE_BINARY_DIR}/external/win/vulkan/Bin/glslc.exe" COPYONLY)
else()
  # NOTE: not required on linux anymore, but update code to reflect that
  configure_file("${CMAKE_SOURCE_DIR}/external/vulkansdk/glslc" "${CMAKE_BINARY_DIR}/external/vulkansdk/glslc" COPYONLY)
endif()
