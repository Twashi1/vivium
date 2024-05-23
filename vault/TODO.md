
## High priority

- All metadata of Vulkan resources must just be mostly trivial
	- Can move by just copying bytes, and setting old ones to 0 (trivial move?)
- Somehow deal with Vivium resources that contain multiple Vulkan resources (shaders)
- Create `Commands::createPipeline` and similar methods, and make them take allocation callbacks
## Core

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
## Vulkan

- In-place allocation of resources to subvert double pointer indirection
- Make the static resource manager "re-useable", i.e., can `allocate` multiple times
- Shader debugger tool - use CPU to simulate GPU actions for some fragments
- Compute shaders and storage images (alternative to framebuffers?)
- Submit calls should take memory to fill instead of returning a vector
- Dynamic resource manager

## GUI

- `Sprite` class (load similar sprites from texture atlas, data structure is difficult to think of)
- `Slider` class
- `Panel` class
- `Anchor` renamed since also used in `Center` parameters (also move to `Vec2`?)
- Not considering the total y-extent of characters that go below the origin (like `p`, `q`, `y`, etc.), although whether it should be considered or not is to be determined
## Minor

- Lots of methods missing validation of pointers
- Lots of missing resetting handles to `nullptr`
- Lots of methods call to implementation in `Resource`
- Lots of things that should be `uint32_t` instead of `float` (in particular with respect to dimensions)
- Work on cleaning up some warnings whenever bored
- Use `maxLineWidth` of `Text::Metrics` where referenced
- Rename private functions with underscore prefix
- `Math::calculateAlignment` brought back in some form, under new name with new structure
- Use `VIVIUM_DEBUG_LOG` instead of `VIVIUM_LOG` when we only want to log in `DEBUG` mode

Resources contain metadata, at the end of the resource, the Vulkan resource itself is stored. In order to create a resource, with our current model, we would be required to know the size of the Vulkan resource, so we can place the Vivium resource in memory. We wouldn't be allowed to hold references in the form of Handles to uninitialized resources, since they don't exist yet. Instead, we hold references to the specification? Then when the resource is created in the previous step, we obtain the actual resource itself by looking into the given mapping of specification indices to allocation locations.

Steps of program should be:
- Create vulkan allocation
- Place metadata after (requires passing size of metadata to ensure a contiguous region can be allocated)
- Assume we get back the location of the allocation (safe to assume this is just the Vulkan resource itself?), so that we can map the specification index to the location of the allocation

All vulkan resource allocations require an additional piece of metadata, the size of the vulkan resource + alignment, so that we can accurately determine the beginning of the metadata given the vulkan resource pointer. This additional piece of metadata is also required for the re-allocation function

Must investigate the assumption that the vulkan handle is equivalent to the pointer given for memory allocation

For reallocation, we must be able to ascertain the total size of the previous allocation (excluding metadata), including where that is stored, given only the pointer of the allocation. Thus the first 4 bytes before the allocation pointer, we should define to store the size of the previous allocation (but then alignment struggles, need to grab 4 bytes from somewhere)