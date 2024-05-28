
## High priority

- Make everything a handle again
## Core

- Platform independence (OS module, Timer module)
- Test multi-window draws
	- Requires multi-window application flow
- `inl` files for all templates
- Test physics modules
- Span synonymous type
	- `initializer_list` compatible
- Vector synonymous type
- Rust-style errors (try to make `Option` efficient?)
- Clear render/GUI/computation threads. Ability to submit from multiple threads
- Using filesystem or better file referencing (not just passing file data, but truly passing file path in a way that guarantees the existence of that path)
- Dynamic tree allocator
- Methods for `Input` to be more concise
- `VIVIUM_LOG_PERIODIC(interval, severity, message, ...)`
- All allocated resources should be tracked in debug mode (regardless of static/dynamic or even type of allocator, need some intermediary registry) - use `VkAllocationCallbacks` for this
## Vulkan

- Generalise `Commands::createRenderPass` for `Framebuffer` render passes as well
- Make the static resource manager "re-useable", i.e., can `allocate` multiple times
- Shader debugger tool - use CPU to simulate GPU actions for some fragments
- Compute shaders and storage images (alternative to framebuffers?)
- Dynamic resource manager

## GUI

- Reduce parameters on GUI visual, customiseability not the point of the system
- `Scene` rendering for batching
- `Button` functionality (hover/click events, colour changes on hover/click)
- `Slider` class
- `Panel` class
- `Anchor` renamed since also used in `Center` parameters (also move to `Vec2`?)
- Not considering the total y-extent of characters that go below the origin (like `p`, `q`, `y`, etc.), although whether it should be considered or not is to be determined
- Better values of `spreadFactor` for signed distance field font rendering
## Minor

- Significantly better debug checks (on things like `Batch` for example)
- Use internal shaders/fonts, not external
- Lots of methods missing validation of pointers
- Lots of missing resetting handles to `nullptr`
- Lots of methods call to implementation in `Resource`
- Lots of things that should be `uint32_t` instead of `float` (in particular with respect to dimensions)
- Work on cleaning up some warnings whenever bored
- Use `maxLineWidth` of `Text::Metrics` where referenced
- Use `std::string_view` where applicable
- Rename private functions with underscore prefix
- Use `VIVIUM_DEBUG_LOG` instead of `VIVIUM_LOG` when we only want to log in `DEBUG` mode
- `T const&` a lot of things 
## Aspirational

- 3D workflow (camera + controller math)
- Raytracing
## Projects

- Simple shoot-em-up
- Circuit simulator