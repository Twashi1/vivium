cmake_minimum_required (VERSION 3.24)

project ("vivium4" LANGUAGES C CXX)

# TODO: don't know if required
include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(GLFW3_VERSION 3.4)
set(VULKAN_VERSION v1.4.326)
set(FREETYPE_VERSION VER-2-13-3)
set(LUA_VERSION v5.4.8)
set(LUA_VERSION_MIN 5.4)

find_package(glfw3 ${GLFW3_VERSION} QUIET)
if (NOT glfw3_FOUND)
  message(STATUS "Fetching GLFW3")

  FetchContent_Declare(
    glfw3
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG ${GLFW3_VERSION}
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
  )

  set(GLFW_INSTALL OFF)
  set(BUILD_SHARED_LIBS ON)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_WAYLAND OFF)
  set(GLFW_BUILD_COCOA OFF)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(glfw3)
endif()

# TODO: use find_package for all?
find_package(Vulkan COMPONENTS glslc)
if (NOT Vulkan_FOUND)
  message(FATAL_ERROR "Failed to find Vulkan, install the SDK following README.md")
endif()

find_package(Freetype)
if (NOT Freetype_FOUND)
  message(STATUS "Fetching Freetype")

  FetchContent_Declare(
    Freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG ${FREETYPE_VERSION}
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
  )

  FetchContent_MakeAvailable(Freetype)
endif()

# TODO: pretty sure if LUA is found, this wont run correctly
#find_package(Lua ${LUA_VERSION_MIN}) 
if (TRUE)
  message(STATUS "Fetching Lua")

  FetchContent_Declare(
    lua    
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG ${LUA_VERSION}
    GIT_PROGRESS TRUE
  )

  FetchContent_MakeAvailable(lua)

  file(GLOB LUA_SOURCES "${lua_SOURCE_DIR}/*.c")
  list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/onelua\\.c$")
  add_library(lua STATIC ${LUA_SOURCES})
  target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})

  if (MSVC)
    target_compile_definitions(lua PRIVATE _CRT_SECURE_NO_WARNINGS)
  else()
    target_compile_options(lua PRIVATE -O3)
  endif()
endif()

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

add_executable(runtime ${VIVIUM_HEADERS} ${VIVIUM_SOURCES} ${RUNTIME_HEADERS} ${RUNTIME_SOURCES} "runtime/main.cpp")
add_executable(editor ${VIVIUM_HEADERS} ${VIVIUM_SOURCES} ${EDITOR_HEADERS} ${EDITOR_SOURCES} "engine/main.cpp")
add_executable(test ${VIVIUM_HEADERS} ${VIVIUM_SOURCES} ${TESTING_HEADERS} ${TESTING_SOURCES} "testing/main.cpp")
add_executable(verlet_run ${VIVIUM_HEADERS} ${VIVIUM_SOURCES} ${VERLET_HEADERS} ${VERLET_SOURCES} "verlet/main.cpp")

target_compile_features(runtime PUBLIC cxx_std_20)
target_compile_features(editor PUBLIC cxx_std_20)
target_compile_features(test PUBLIC cxx_std_20)
target_compile_features(verlet_run PUBLIC cxx_std_20)

string(LENGTH "${CMAKE_SOURCE_DIR}/" VIVIUM_SOURCE_PATH_SIZE)
message(VIVIUM_SOURCE_PATH_SIZE="${VIVIUM_SOURCE_PATH_SIZE}")
add_definitions("-DVIVIUM_SOURCE_PATH_SIZE=${VIVIUM_SOURCE_PATH_SIZE}")

# Header-only, build same regardless of system
target_include_directories(runtime PUBLIC "${CMAKE_SOURCE_DIR}/external/glm")
target_include_directories(runtime PUBLIC "${CMAKE_SOURCE_DIR}/external/stb_image")

target_include_directories(editor PUBLIC "${CMAKE_SOURCE_DIR}/external/glm")
target_include_directories(editor PUBLIC "${CMAKE_SOURCE_DIR}/external/stb_image")

target_include_directories(test PUBLIC "${CMAKE_SOURCE_DIR}/external/glm")
target_include_directories(test PUBLIC "${CMAKE_SOURCE_DIR}/external/stb_image")

target_include_directories(verlet_run PUBLIC "${CMAKE_SOURCE_DIR}/external/glm")
target_include_directories(verlet_run PUBLIC "${CMAKE_SOURCE_DIR}/external/stb_image")

target_link_libraries(runtime PRIVATE Vulkan::Vulkan)
target_link_libraries(runtime PRIVATE Freetype::Freetype)
target_link_libraries(runtime PRIVATE glfw)
target_link_libraries(runtime PRIVATE lua)

target_link_libraries(editor PRIVATE Vulkan::Vulkan)
target_link_libraries(editor PRIVATE Freetype::Freetype)
target_link_libraries(editor PRIVATE glfw)
target_link_libraries(editor PRIVATE lua)

target_link_libraries(test PRIVATE Vulkan::Vulkan)
target_link_libraries(test PRIVATE Freetype::Freetype)
target_link_libraries(test PRIVATE glfw)
target_link_libraries(test PRIVATE lua)

target_link_libraries(verlet_run PRIVATE Vulkan::Vulkan)
target_link_libraries(verlet_run PRIVATE Freetype::Freetype)
target_link_libraries(verlet_run PRIVATE glfw)
target_link_libraries(verlet_run PRIVATE lua)

# TODO: implement these with custom target
file(COPY "${CMAKE_SOURCE_DIR}/vivium4/res" DESTINATION "${CMAKE_BINARY_DIR}/vivium4/")
file(COPY "${CMAKE_SOURCE_DIR}/engine/res" DESTINATION "${CMAKE_BINARY_DIR}/engine/")
file(COPY "${CMAKE_SOURCE_DIR}/verlet/res" DESTINATION "${CMAKE_BINARY_DIR}/verlet/")

# TODO: need to redo-this and find the glslc binaries from the correct location
# Probably need to redo this
# Vulkan_GLSLC_EXECUTABLE
if (WIN32)
  configure_file("${CMAKE_SOURCE_DIR}/external/win/vulkan/Bin/glslc.exe" "${CMAKE_BINARY_DIR}/external/win/vulkan/Bin/glslc.exe" COPYONLY)
else()
  # NOTE: not required on linux anymore, but update code to reflect that
  configure_file("${CMAKE_SOURCE_DIR}/external/vulkansdk/glslc" "${CMAKE_BINARY_DIR}/external/vulkansdk/glslc" COPYONLY)
endif()
