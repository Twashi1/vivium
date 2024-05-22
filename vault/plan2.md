
Look towards custom allocator (memory aligned, etc.) to remove double pointer indirection
## Requirements

- Heavily data-oriented style
	- No member functions
	- Clear separation of API-related functions, and general private functions (e.g. GLFW uses underscore prefix)
- No move semantics are left to user, structs are pure-data (with RAII where required)

## TODO

### Core

- Platform independence (OS module, Timer module)
- `inl` files for all templates
- Test physics modules
- Span synonymous type
- Vector synonymous type
- Convert text functions to use `std::string_view`
- Rust-style errors (try to make `Option` efficient?)
- Clear render/GUI/computation threads. Ability to submit from multiple threads
- Using filesystem or better file referencing (not just passing file data, but truly passing file path in a way that guarantees the existence of that path)
- Dynamic tree allocator
- Methods for `Input` to be more concise
- `VIVIUM_LOG_PERIODIC(interval, severity, message, ...)`
- All allocated resources should be tracked in debug mode (regardless of static/dynamic or even type of allocator, need some intermediary registry)
### Vulkan

- In-place allocation of resources to subvert double pointer indirection
- Make the static resource manager "re-useable", i.e., can `allocate` multiple times
- Shader debugger tool - use CPU to simulate GPU actions for some fragments
- Compute shaders and storage images (alternative to framebuffers?)

### GUI

- `Sprite` class (load similar sprites from texture atlas, data structure is difficult to think of)
- `Slider` class
- `Panel` class
- `Anchor` renamed since also used in `Center` parameters (also move to `Vec2`?)
- Fix `Font` SDF generation padding (also memory leak likely)
### Minor

- Lots of methods missing validation of pointers
- Lots of missing resetting handles to `nullptr`
- Lots of methods call to implementation in `Resource`
- Lots of things that should be `uint32_t` instead of `float` (in particular with respect to dimensions)
- Work on cleaning up some warnings whenever bored
- Use `maxLineWidth` of `Text::Metrics` where referenced
- Rename private functions with underscore prefix