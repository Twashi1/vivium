## Whiteboard

Reached practical limitations of GUI
- just force all the features in and continue, brute-force through it
- we just can't afford a GUI re-write

How do we organise the game engine?
- ideally we want to create something unique
- and with minimal overhead
- ideal would be to directly compile to `C++` the construction of objects
	- but seems complicated in practice?
- better would be to compile to some intermediary
	- optimise/interpret this intermediary representation
	- can then compile intermediary in future, but for now direct interpretation seems much easier

How does the user interact with the game engine?
- user creates entities
- user is responsible themselves for creation of the pipeline
	- they construct and attach shaders
	- any relevant shader resources are attached to a descriptor set, which is then attached to the pipeline with a matching descriptor layout
- representation of draw commands?
	- need some submission of a buffer to the shader layout/for rasterization
	- any given pipeline also has a buffer layout that the buffer must match
	- but we just also attach the buffer to the pipeline
	- when we draw, we have to select the relevant buffer/descriptor to bind

To create
- ability to add different entities
- modify individual instances with given properties
	- enter integer, string, list data
- drag and drop to add to hierarchy (like add element to inspector window)
- expand/hide child elements
- given some tree of children, interpret it to create elements
	- parse the tree somehow?

How does rendering properties, entering properties, and creating the properties menu/storing properties for some type of object work
- for the game engine we'd organise `Vulkan` objects as their own individual components
- each component has a set of values we'd need to modify
	- we hardcode the UI for these components?
	- we have list/string entries
		- we need this list UI to take arbitrary data
		- we need some drop-down menu functionality for selecting the `ShaderDataType`
		- list UI has a generalised display/entry mechanism (plus remove/organise buttons ideally)
	- the UI itself stores the values for these components?

We don't actually have a way to change a container's size based on its children

To build a real UI quickly an efficiently
- need some language with which to describe the UI quickly
- the language must incorporate almost any expected feature very easily
- intuitively build UI hierarchy

Say we were to re-design the UI around a different philosophy
- vast majority of UIs have a set font size and size of elements, with areas changing shapes to fit
- we can support the sizing thing, but we can't modify areas to change shape to fit
- we also can't make a container base its size off its children
- "fit sizing" -> "grow sizing" -> "positions" -> "draw"
- break up GUI update function a little to make it simpler
- clipping of text? more text functions for organising and controlling text

We use the ECS to store most relevant data
- each entity has components for what data it has on it
- pipeline objects require links
	- buffer specification
	- shaders
	- etc.
- we create these links by children or by drag and drop
	- just look at each child of the tree container, get the relevant entity for that panel
	- get the components for that entity
	- see which ones fit our needs for that pipeline's links
- an entry can link to a different entity?

## Shader planning

- run-time reflection and some partial compilation on shaders
	- checks alignment requirements
- vertex/fragment shader merging (can look into geometry/tesselation/compile/etc. later)
- code re-use across shaders with utility files and such
	- c-like include structure

- might as well look into building a LSP for it
- look into debug and simulation on CPU side (would require rasterization etc.)
## Current tasks

- CMAKE of vivium should be separate to CMAKE of editor
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

- Iterators for single component view
- Drop function for group view
- Non-ownership group relationship optimisations?
- Emplace/replace component
- Add copy of component
- Permanent `T**` (if wanted)
- Investigate ability to change size of group (not during iteration) and still correctly see all entities
## Core

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
- Super easy `debugRect` and `debugPoint` commands for a given coordinate or GUIElement
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

## Future

> After the MVP works

Code style is torn between attempts to be C-compatible and a data-oriented C++ style
- We should stick to one
	- Namespaces? Just include them in the name of the relevant function
	- In-code templates? None are a strict necessity, but seem to be needed to avoid excessive code duplication
	- STL usage? We really need `std::vector`, not really any alternative other than macro programming which would make us completely C project
	- We either lean more closely into the C++ style, or we pick a different language (or make custom)
		- Consequences: reverting back to using namespaces to split up code
		- Large parts of code change to fit C++ style

GUI is an inefficient mess
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

```
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
