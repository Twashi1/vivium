## Whiteboard

- Need abstraction to be able to efficiently develop apps
- Standard UI elements
	- Slider
- Easy-draw elements
	- Standard shaded quad (ability for texture, standard coordinates plugged in, etc.)

## Current tasks

- Some GUI commands are randomly split between `context.h` and `base.h`
- 3D rendering tests (for fun)
- HTML renderer
- Dynamic resource manager
- Input class refactor
	- Initialisation function and update
- Font namespace
	- Initialisation function
- Math class namespaces
- Check non-multisampled rendering is still working
- Serialiser is super old
- Test framebuffers
- Resize-able framebuffers?
- Shared command pool for framebuffers?
## ECS

- Iterators for single component view
- Drop function for group view
- Non-ownership group relationship optimisations?
- Emplace/replace component
- Add copy of component
- Permanent `T**` (if wanted)
- Investigate ability to change size of group (not during iteration) and still correctly see all entities
## Core

- Dynamic allocation storage (at least a wrapper for `new`/`delete`  temporarily)
- Reflection data on shader files
	- Use to validate alignment requirements
	- Some custom parsing of shader files
- Offset before size on `PushConstant`, not ideal
- Can free `DescriptorLayout` at same stage we free `Shader`?
- Static allocator being re-useable is an anti-pattern
- Test multi-window draws
	- Requires multi-window application flow (around the engine creation mostly)
- Abstract creation functions of resources to maximum extent without compromising performance
- Fix `beginFramebufferFrame` and `endFramebufferFrame`, and plan easy render-target setting
- Re-implementation of dynamic buffers
- Minimal overrides
- No static member functions (even for specifications): just define a method
- Renaming `ResourceManager` to `ResourceAllocator`
- `Commands::Context` should be multi-thread compatible
- Easier-to-use temporary staging
	- `Stager s = Commands::createStage(maximumSize)`
	- `Commands::setStageData(s, ...)`
	- `Commands::uploadStage(s, bufferSlice, ...)`
	- `Commands::freeStage(s)` (issues with `VkDeviceIdle` possible - schedule with a `Context` instead?)
- `inl` files for all templates
- Error system
- Clear separation render/GUI/computation threads. Ability to submit from multiple threads
- Resource management system
	- Custom file path syntax for loading resources (OS agnostic ideally)
- Dynamic resource allocation
	- Dynamic tree allocator
- Methods for `Input` to be more concise
- `Buffer::Slice(buffer)` - ability to allocate multiple slices within a buffer as a more aspirational goal
	- Update `transferBuffer` to use slices
	- Should be very quick to create from a buffer (almost seamless)
## Vulkan

- Generalise `Commands::createRenderPass` for `Framebuffer` render passes as well
- Initialiser lists for resource manager
	- Add functions that take initialiser lists
- Compute shaders and storage images (alternative to framebuffers?)
- Dynamic resource manager
- Vertex input rate: `VK_VERTEX_INPUT_RATE_INSTANCE`
- Shaders should not be created through `compileShader`, at least not internally

## GUI

- Reverse argument order of `Text`
- Cannot have multiple `renderPanel` or `renderButton` calls in the same draw call, data gets overwritten
- Test left-aligned `Text` rendering
- Per character `Text` colouring
- Reduce parameters on GUI visual, customisability not the point of the system
- `Scene` rendering for instanced/batching
- `Sprite` class
- `Button` functionality (hover/click events, colour changes on hover/click)
	- Add ability for `Sprite` to display
	- Should have `TextBatch` in `GUIContext` so we don't need a draw call for each button
- `Slider` class
- `Anchor` renamed since also used in `Center` parameters (also move to `Vec2`?)
- Not considering the total y-extent of characters that go below the origin (like `p`, `q`, `y`, etc.), although whether it should be considered or not is to be determined - consider a line spacing parameter
- Better values of `spreadFactor` for signed distance field font rendering

## Physics

- Sub-steps
- K-D Trees
- Minimise transformations on bodies
## Minor

- `VIVIUM_LOG_PERIODIC(interval, severity, message, ...)`
- 8-bit index buffers
- Determine whether or not bindings of resources are per shader stage, or shared
- Significantly better debug checks (on things like `Batch` for example)
- Lots of methods call to implementation in `Resource`
- Lots of things that should be `uint32_t` instead of `float` (in particular with respect to dimensions)
- Work on cleaning up some warnings whenever bored
- Use `maxLineWidth` of `Text::Metrics` where referenced
- Use `std::string_view` where applicable
- Rename private functions with underscore prefix
- `math.h` and `math.cpp` contain wide variety of functions, mixing of texture indexing maths and camera maths
- `T const&` a lot of things
- `BufferReference::memoryIndex` should be an enum
- Vulkan still prints sometimes

## Possible

- Consider a less literal usage of `const`, where `const` does apply to objects whose GPU/host memory is being modified
- Consider a model where we don't return any values or rarely return values, instead opting for a result code for functions that could error
## Aspirational

- 3D workflow (camera + controller math)
- Raytracing
- Shader debugger tool - use CPU to simulate GPU actions for some fragments
- Platform independence (OS module, Timer module)
