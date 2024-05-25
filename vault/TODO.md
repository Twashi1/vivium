
## High priority

- All metadata of Vulkan resources must just be mostly trivial
	- Can move by just copying bytes, and setting old ones to 0
- Move creation of render pass to `Commands::createRenderPass`
## Core

- Platform independence (OS module, Timer module)
- `inl` files for all templates
- Test physics modules
- Span synonymous type
- Vector synonymous type
- Rust-style errors (try to make `Option` efficient?)
- Clear render/GUI/computation threads. Ability to submit from multiple threads
- Using filesystem or better file referencing (not just passing file data, but truly passing file path in a way that guarantees the existence of that path)
- Dynamic tree allocator
- Methods for `Input` to be more concise
- `VIVIUM_LOG_PERIODIC(interval, severity, message, ...)`
- All allocated resources should be tracked in debug mode (regardless of static/dynamic or even type of allocator, need some intermediary registry)
## Vulkan

- Vivium resources that contain multiple vulkan handles should allocate them together (pass additional context to achieve this)
- Make the static resource manager "re-useable", i.e., can `allocate` multiple times
- Shader debugger tool - use CPU to simulate GPU actions for some fragments
- Compute shaders and storage images (alternative to framebuffers?)
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
- `T const&` a lot of things 

```
Header: u32 size, u16 metadataSize

(alignment)
-Header:  Header
0:    PrimaryVulkanResource
size: Metadata (contains pointers to additional resources)
```

We have no way to guarantee additional resources are allocated contiguously with the primary, unless we gather all resources to be allocated with that particular object first (which is just impractical)

Any such additional resources, we relegate to being allocated randomly by the Vulkan allocator

```
vector<ResourceReferences> references = submit(ResourceManager, specifications)
// Where the specifications take references to some previous resources
vector<ResourceReferences> secondary = submit(ResourceManager, specifications + references);

allocate();

vector<handles> handles = grab(references)

draw/use/copy (handles)
```