
## Requirements

- Heavily data-oriented style
	- No member functions
	- Clear separation of API-related functions, and general private functions (e.g. GLFW uses underscore prefix)
- Classes are pure data, if any functionality other than a copy is required, it is done through a function - not a constructor
- Try to make even the static resource manager "re-useable", i.e., can `allocate` multiple times

## General

- Span synonymous type
- Vector synonymous type
- Using filesystem or better file referencing (not just passing file data, but truly passing file path in a way that guarantees the existence of that path)
## Types

### Resources

#### Memory allocated

- Buffer
- Dynamic buffer
- Texture
- Descriptor set

All these resources will have be created through submission to a resource manager, returning a `PromisedHandle`, which can be assumed to be a `Handle` after calling the allocation of a resource manager.

#### Trivially constructed (no drop)

- Uniform binding
- Texture atlas

All just use a pure constructor

#### Non-trivially constructed (no drop)

- Buffer layout
- Shader specification
- Texture specification
- Font

*In the case where copies are potentially expensive, or non-trivial, what do we do?*
- Designing the class such that there is no requirement for non-trivial copy, and then simply allowing expensive copies, encouraging by-reference passing. Potentially also allowing allocation of any resource on our allocators/storage.
- This is done by using RAII structures like `vector` and `string`

Buffer layout type defines a static constructor function.
Shader specification defines static constructor function(s).
Texture specification defines static constructor function(s).
Font defines static constructor function(s).

TODO: Shader itself doesn't need to be allocated, and can instead just be a pure data class we copy as we wish, with the restriction of one drop. Maybe as convention, anything that strictly requires a `drop`, especially in relation to a Vulkan resource, should be a `Handle/Resource` combo
#### Other

- Shader (created at any point)
- Pipeline (relies on valid handles, and relies on memory for specification existing - inconvenient without cleanup)
- Descriptor layout (created at any point)

Shader, Buffer layout, Descriptor layout can all be constructed through `create` functions, returning a `Handle`

Pipeline is allocated on a resource manager, despite the lack of a need to create memory to ensure correct resource creation order.

#### Higher-level resources

*i.e.* Text, Batch, resources composed of other resources

Goal: One submission call to a resource manager, after which they are either `PromisedHandle` or `Handle`

Thus, any object that relies on valid handles for creation, must be something submitted to a resource manager to control resource creation order.

If an object returns a `PromisedHandle`, it must be through a `submit` function, otherwise it must be through a `create` function (Any `handle`-returning resource would have to be instantly valid on call, thus only contains instant-available types)
## GUI

The GUI base type, allowing for hierarchical structure and relationship-based positioning. Split into the pure-functionality of the `Base` type, and the various provided types in the `Visual` namespace (with very limited customisability)

Exemplar use of `Base` is well-defined by the existing `Visual` types (hopefully)

*4 namespaces, Vivium, GUI, Visual, Button, quite excessive*
Examples of `Visual` usage:

```cpp
GUI::Visual::Button::Handle button = GUI::Visual::Button::submit(resourceManager, engine, ...);

if (GUI::Visual::Button::isPressed(button))
	// Do something

GUI::Visual::Button::render(button, ...);
```

The rendering side makes heavy sacrifices on performance, with a draw call for each and every GUI element. Since the visual system is inflexible by design, wouldn't be impossible to convert it entirely into some instanced/batched single draw call.
## Errors

Likely want to adapt a Rust-like approach, only `Fatal` errors, or `Option` return values