
## High priority

- Offset before size on `PushConstant`, update old systems
- Double deconstructor call on `Storage::dropResource`
- Update `Batch` and `GUI` to new system
	- Remember `Batch::setup` now required to be called
- Not all `drop` methods have correct naming convention
- Some clean-up method on `ResourceManager::Static` that drops all resources once user is ready
## Core

- Can free `DescriptorLayout` at same stage we free `Shader`?
- Static allocator being re-useable is an anti-pattern
- Test multi-window draws
	- Requires multi-window application flow (around the engine creation mostly)
- Abstract creation functions of resources to maximum extent without compromising performance
- Most `isNull` methods are only relevant in degenerate use-cases?
- Fix `beginFramebufferFrame` and `endFramebufferFrame`, and plan easy render-target setting
- Re-implementation of dynamic buffers
- Minimal overrides
- No static member functions (even for specifications): just define a method
- `Math::orthogonalPerspective2D` should not be taking `Window::Handle`, but a dimension
- Renaming `ResourceManager` to `Allocator::Static`
- RAII storage objects are appealing (delete copy, define move) (`Storage::Static` and `Storage::Dynamic`)
	- Alternative is to just `malloc` and `free` these, they're already only used as pointers anyway
- `Commands::Context` should be multi-thread compatible
- Easier-to-use temporary staging
	- `Stager s = Commands::createStage(maximumSize)`
	- `Commands::setStageData(s, ...)`
	- `Commands::uploadStage(s, bufferSlice, ...)`
	- `Commands::freeStage(s)` (issues with `VkDeviceIdle` possible - schedule with a `Context` instead?)
- `inl` files for all templates
- Span synonymous type (`Slice`)
	- `initializer_list` compatible (seems impossible)
- Vector synonymous type
- Rust-style errors (try to make `Option` efficient?)
- Clear render/GUI/computation threads. Ability to submit from multiple threads
- Using filesystem or better file referencing (not just passing file data, but truly passing file path in a way that guarantees the existence of that path)
	- Resource management system
- Dynamic tree allocator
- Methods for `Input` to be more concise
- All allocated resources should be tracked in debug mode (regardless of static/dynamic or even type of allocator, need some intermediary registry)
- `Buffer::Slice(buffer)` - ability to allocate multiple slices within a buffer as a more aspirational goal
	- Update `transferBuffer` to use slices
	- Should be very quick to create from a buffer (almost seamless)
## Vulkan

- Test re-useable resource manager
- Generalise `Commands::createRenderPass` for `Framebuffer` render passes as well
- Initialiser lists for resource manager
	- Add functions that take initialiser lists
- Compute shaders and storage images (alternative to framebuffers?)
- Dynamic resource manager
- Vertex input rate: `VK_VERTEX_INPUT_RATE_INSTANCE`
- Check for `DEVICE_LOCAL`, not whether it is `UNIFORM` or `DEVICE`

## GUI

- Reduce parameters on GUI visual, customisability not the point of the system
- `AddChild` public method
- `Scene` rendering for instanced/batching
- `Button` functionality (hover/click events, colour changes on hover/click)
- `Slider` class
- `Panel` class
- `Anchor` renamed since also used in `Center` parameters (also move to `Vec2`?)
- Not considering the total y-extent of characters that go below the origin (like `p`, `q`, `y`, etc.), although whether it should be considered or not is to be determined
- Better values of `spreadFactor` for signed distance field font rendering
- `List` type is still poorly integrated and not tested

## Physics

- Sub-steps
## Minor

- `VIVIUM_LOG_PERIODIC(interval, severity, message, ...)`
- 8-bit index buffers
- In-place allocation where possible (use `Resource` internally more often)
	- Ability to `Inplace` statically allocated resources (involves passing allocator type to `ResourceManager::Static` functions)
- Determine whether or not bindings of resources are per shader stage, or shared
- Significantly better debug checks (on things like `Batch` for example)
- Lots of methods missing validation of pointers
- Lots of missing resetting handles to `nullptr`
- Lots of methods call to implementation in `Resource`
- Lots of things that should be `uint32_t` instead of `float` (in particular with respect to dimensions)
- Work on cleaning up some warnings whenever bored
- Use `maxLineWidth` of `Text::Metrics` where referenced
- Use `std::string_view` where applicable
- Rename private functions with underscore prefix
- Use `VIVIUM_DEBUG_LOG` instead of `VIVIUM_LOG` when we only want to log in `DEBUG` mode
	- should still have customisable warning level
- `T const&` a lot of things

## Possible

- Minimal namespaces (only `Vivium`), instead just write everything out; `bufferDrop` instead of `Buffer::drop`
	- `Storage::Static` and `Storage::Dynamic` no longer have a clear separation, would have to integrate it in the name somehow
	- `Buffer::Dynamic` and `Buffer` need to be integrated into the name
## Aspirational

- 3D workflow (camera + controller math)
- Raytracing
- Shader debugger tool - use CPU to simulate GPU actions for some fragments
- Platform independence (OS module, Timer module)
## Projects

- Simple shoot-em-up
- Circuit simulator

# Game

## General

- Enemy spawning
- Values should be given in units, not relative to size of monitor?