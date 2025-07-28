## Next

- scripting language
	1. we need implementations of each of the relevant commands laid out in [[Script]]
		- we need to preserve information about the entity tree so we can accurately reference entities
		- thus we need an arbitrary way to store entities and their components (serialiser)
		- we also need to be able to rename entities (change entity tree to use entry boxes?)
- need to be able to re-open a project in the editor (save/load functionality)
- we allow multiple imports of vivium to avoid some useless shared middleman header
- we need the scroll bar

Big note for serialiser
- in some places, we assume the passed pointer points to nothing, so we can just treat it as store for memory for us to create
- or, we create the assumption the pointer always points to some valid memory for the object, and thus we use assignment to delete the old object/replace it at that pointer

Serialisation format changes
- serialise function for c-array?
1. we want to capture most of the entity tree (child entities and such don't matter)
2. we want to preserve the data in the entries, not the ones in the components
3. lua submit should be able to modify where data is gotten from by changing the entity, and also change the data entered; or fill in the date entered
4. lua needs an easy memorable way to reference entities (names and even ids will work; but most systems use child scripts referencing their parents, or parameters entered into scripts)

5. at run-time, without information of the original type, we need to be able to serialise each component pool
	- so the component manager must define a pointer function to convert the component into binary data (by default, SerialiserWrite)
		- we don't want the user to have to define anything other than `serialiseWrite` and `serialiseRead` for each component
		- we cannot account for the `SerialiserInterface` argument... just give up and make it a file interface always...?
	- ideally this pointer function can write directly to an interface?
	- for this it needs to be cable of taking a functor which requires templating?
	- alternatively it writes to an intermediary, which is then directly written to the serialiser
6. at run-time, with the information of the original type, we should be able to deserialise this information as needed


What we really want, is a way to serialise the ECS registry

An entity passes to descriptor set can have both a buffer/texture/framebuffer, and thus there is ambiguity as to which should be taken as the desired data
- also this entity-passing system seems flawed?
- should it not instead be that the components themselves are organised in the tree structure for inheritance

## Shader planning

- run-time reflection and some partial compilation on shaders
	- checks alignment requirements
- vertex/fragment shader merging (can look into geometry/tesselation/compile/etc. later)
- code re-use across shaders with utility files and such
	- c-like include structure

- might as well look into building a LSP for it
- look into debug and simulation on CPU side (would require rasterization etc.)
## Current tasks

- Buffer component has `size` unused (should be number of elements?) also can't take arbitrary data
- CMAKE of vivium library should be separate to CMAKE of editor/runtime
- Comprehensive documentation of all structs/methods/etc.
	- just use doxygen format, can build a custom tool later
- Some GUI commands are randomly split between `context.h` and `base.h`
- 3D rendering tests (for fun)
- HTML renderer
- Dynamic resource manager
- Input class refactor
	- Initialisation function and update
	- Some easier way to get text input? Dealing with all backspace/modifiers/etc
- Physics namespaces
- Check non-multisampled rendering is still working
- Serialiser is super old
- Test framebuffers
- Resize-able framebuffers?
- Shared command pool for framebuffers?
- Scrollable containers/scroll bar
## ECS

- fundamental issue with attempting to serialise the ECS
	- we use pointer functions based on a passed-type to manage the components
	- without the type to get these pointer functions, we cannot serialise the components safely

- opt.1 serialisation function in the component manager
	- the manager uses the serialisation function to serialise each component
	- to deserialise, we simply store the serialised data until that component type is registered again
		- at which point we can properly deserialise the data and re-enable that component pool

- nothing better?

- we already place a restriction on the number of components per registry
	- additionally restrict number of registries?
	- any component id is based on a family type generator
	- we want a simple mapping from component id to an index within a registry
	- sparse set can do this in O(1)?

- Iterators for single component view
- Drop function for group view
- Non-ownership group relationship optimisations?
- Emplace/replace component
- Add copy of component
- Permanent `T**` (if wanted)
- Investigate ability to change size of group (not during iteration) and still correctly see all entities
## Core

- we need scripts, and to be able to control certain aspects through scripts
	- need a scripting language
	- looking closest towards using LUA?
	- but a custom scripting language is also an option (more fun too)
		- more easy to integrate in the end maybe
	- most functionality would be integrated by built-in functions into the language itself
	- when interpreting this language, its possible we can find some advantages in how resources are allocated? not having to worry about order so much because the interpreter sorts it out for us beforehand
- certain pieces of data entry are just too cumbersome to do by hand
	- we could do them by upload entries?
- rename entities
- see explicit name of each component, bordered and containerised
- We assign `BufferComponent` and `ShaderComponent` etc. to entities
	- but in reality we never store the data required until compile time
	- so there is some big unnecessary complication?
	- although we do still need some components to be able to reference other components that don't exist
	- which is facilitated by entities right now...
	- so its ok?
- Buffer component shouldn't have size? just get from the data
- We need to be able to smoothly switch between fonts appropriate to the size of text?
- Eventually switch to some truly generalisable system for rendering any type of GUI component
	- only need rectangles for base?
	- then draw any sprites?
	- then draw any text?
	- currently we must impose a strict ordering on what gets drawn first between buttons, sliders, panels, etc.
- Bad naming convention with submit on both submitting resources for rendering and a type of creation
- Possible philosophy change
	- either fully embrace C, or use only C++ conventions
	- this would mean going back to using namespaces to differentiate things
	- allowing member functions?
	- allowing static functions?
- Rename border size px, because its not in terms of pixels but percentage
- Current problem with GUI
	- We made a container, a child of another container
	- we want this child to have no set size, to grow dynamically with the size of its children
	- but the children depend on the size of the container
	- we can make this container span the maximum size
	- but then the next child will be placed way too far down
	- we can make the extent of an element based on only its children
	- but this doesn't fix anchoring onto another container
		- but honestly good enough fix for now?
- Vec2 uses a bunch of static functions
- When creating GUI object, don't add to parent automatically, must add manually to make relationships clearer
- Some elements being added twice (look element index 7 has 2 instances of index 30?)
	- debug mode safety warning?
- Container's need to be updated
- Textures loading upside down for stitched atlas specifically?
- Super easy `debugPoint` command for a given coordinate or GUIElement
- Should be easy to perform event `onButtonPress`, either through callback or looping on an `if`
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
	- `fromXXX` constructors violate this
	- `Color::multiply`
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
- Reduce compile time and get rid of a lot of unnecessary code with `module`?
## Vulkan

- Generalise `Commands::createRenderPass` for `Framebuffer` render passes as well
- Initialiser lists for resource manager
	- Add functions that take initialiser lists
- Compute shaders and storage images (alternative to framebuffers?)
- Dynamic resource manager
- Vertex input rate: `VK_VERTEX_INPUT_RATE_INSTANCE`
- Shaders should not be created through `compileShader`, at least not internally

## GUI

- Rename all `submitEntries` methods, and don't use weird pointer span stuff
- Reverse argument order of `Text`
- Test left-aligned `Text` rendering
- Per character `Text` colouring
- Reduce parameters on GUI visual, customisability not the point of the system
- `Scene` rendering for instanced/batching
- `Button` functionality (hover/click events, colour changes on hover/click)
	- Add ability for `Sprite` to display
	- Should have `TextBatch` in `GUIContext` so we don't need a draw call for each button
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

## Script planning

Going to make a scripting language python-esque
- or regardless; we need some planning for the features of the lua/python/custom language
1. Control primarily through the ECS
2. Built-in`submit`, `setup`, `update`, `draw`, `drop` functions, for each stage
- can add/remove components in `submit`
- can reference entities at any point, and grab components on them
- editing of components is also done strictly through built-in commands
	- this way we can manage when buffer updates happen, or when we

```
e = createEntity()
addComponent; removeComponent; editComponent;
```

- lua calls an external function
- this external function must schedule something to be done within one of the commands

## Future

> After the MVP works

Is a compiled game engine actually special?
- most interactions just devolve into writing code for a scripting language that gets compiled to bytecode and ran, just like unreal/unity?

We need some sort of universal object referencing system for the Runtime
- ECS is best approach
- objects reference others by entity
- we don't just store the data associated with an object, but also the vulkan object

Code style is torn between attempts to be C-compatible and a data-oriented C++ style
- We should stick to one
	- Namespaces? Just include them in the name of the relevant function
	- In-code templates? None are a strict necessity, but seem to be needed to avoid excessive code duplication
	- STL usage? We really need `std::vector`, not really any alternative other than macro programming which would make us completely C project
	- We either lean more closely into the C++ style, or we pick a different language (or make custom)
		- Consequences: reverting back to using namespaces to split up code
		- Large parts of code change to fit C++ style

GUI is an inefficient mess
- the point of the GUI is not to build an optimal low-overhead system like most of the engine
	- but rather give less control and performance for ease of creation and use
	- some balance, also current GUI system requires many extra allocations that are always hidden, but take initialisation time and memory costs
- Investigate how immediate-mode rendering really works?
- Actual profiling concerns
	- `calculateTextMetrics` takes a long time as we re-calculate text every frame
	- many draw calls for each text object
	- multiple different pipelines required for each type of GUI object
- Immediate-mode convenient GUI
	- we don't want to have to construct and manage the memory of GUI objects
	- the ideal is to be able to edit a file so we don't have to recompile each time
	- so we should be specifying the GUI in a file
		- largest problem is running code to modify the GUI at run-time
		- can't really have best of both worlds?
- Either:
	- File-based CSS-like approach, but at cost of complications in modifying GUI
	- In-code with no externals

```
Vivium::GUI buttonObject = Vivium::Button("Button text", callbackFunction/lambda);
Vivium::GUI textObject = Vivium::Text("Hello world");

render(buttonObject, ...params);
render(textObject, ...params);
```
- still need to store these objects
- still need to call rendering commands, conditions on if they should be drawn or not
- still need to drop them
- crucially, still need to organise and sort them with containers/etc, manually set up dimensions and anchors, etc.
- upside is more control and likely performance is better

```cpp
beginGUI(styleGuide)

beginContainer(VERTICAL, growDimension());   
ref = button("Click me", fixed(200, 50), position(left, top, left, top, fixed(0, 0)), clickCallback);
ref = text("Text that toggles visibility", fixedDimension(200, 40))
ref = panel("Fit to size", fitDimension(100, 30), styleOverride); // declaring a minimum size
addEvent(onHover, refPanel, callback);
endContainer();

GUI guiManifest = endGUI()

submitGUI(guiManifest)
setupGUI(guiManifest)
renderGUI(guiManifest)
dropGUI(guiManifest)
```
- still a lot of information to declare about positioning, dimension and how dimension changes
	- its ok to have a lot of information to declare, as long as we have strong defaults and the option to inherit certain positioning from a style guide
- almost guaranteed to be less efficient
- either functions have side effects (somewhat violating design), or we need to pass in the parent to every creation function
	- (and now we see it basically arrives at the same design we had before)

either we make a ton of assumptions on positioning, or we give full control and cause more declaration
at the least this new system would require drastically fewer function calls and tracking of data
- could we make entity tree with this new system?

- if we want an editable GUI, we have 2 approaches
	- unlimited size, but dynamic creation of objects and modification of GUI structure
	- limited size, with enabling/disabling elements as required
- in both approaches, we need static references to objects from which can be referenced to modify structure in functions (e.g. add new elements to a container, or enable/disable rendering)
- or extreme complicated approach where we re-generate the entire GUI each time we need modification, just with some caching

ultimate conclusion is that unless we want to limit GUI functionality, we should keep current GUI, but with some additional functionalities
- grow/fit/fixed sizing
- minimum/maximum/ideal sizing
- rewrite GUI to work in just 2 draw calls
	- rendering all generic panels (including borders, rounded corners, etc. customised)
	- rendering all text
- general "Style" component for most GUI objects? either `ButtonStyle`, `PanelStyle`, etc., or just a single `Style`
- GUI should work under dynamic rendering allowing for better customisability in future

## Final goal for MVP

Editor window allows creation of a rendering pipeline through simple drag-and-drops and data entry
- upon pressing a compile button, this data is converting into bytecode
- add some console command that allows running of this bytecode to perform the described render pipeline