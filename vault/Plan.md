## Errors

### Dangerous functions

could do

```c++
auto returnValue;

Result errorCode = dangerousFunction(&returnValue);

if (errorCode == FILE_NOT_FOUND)
	doSomething;
```

or (preferred?)

```c++
auto returnValue = dangerousFunction().except(FILE_NOT_FOUND, [](){func});
```
## Concurrency

Should be explicit about what is thread-safe and what isn't, but onus is on user to build concurrent systems. Maybe you can write a wrapper for a simple `for` loop.

## Input

### Usage

```c++
auto input = Input::Manager(window);

// Every 30 milliseconds, run input.update
auto interval = Time::Interval(30ms, [&](){input.update();})

interval.run();

// If the E key is down
if (input.isDown(Input::KEY_E)) {}
```

```c++
ResourceManager::Static<Allocator::Linear>::Handle manager = ResourceManager::Static<Allocator::Linear>::create(allocator);

ResourceManager::Static::submit(manager, data);
manager.submit(...data);
```

## Time

### Timer

Measures seconds. Same as always

### Interval

Execute a function at a given interval. Larger time intervals can use some sort of thread sleep, shorter ones may need a `nanosleep`. Will require `nanosleep` to be thread-safe.

```c++
Time::Interval command = Time::Interval(5ms, myFunction, callee, userArgs);

// Starts the thread
command.run();

// Async statements run normally
while (gameloop) {}

// Stops the thread (thread.join)
command.stop();
```

## GUI

### Usage

```c++
Allocator allocator;

// An invisible panel, for alignment purposes only
auto windowPanel = GUI::Panel::Create(allocator, GUI::Panel::Specification{});
// Will make window span the entire screen (SPECIAL!)
GUI::setParent(windowPanel, Window::getPanel(window), GUI::Anchor::LEFT | GUI::Anchor::TOP);

auto panel = GUI::InterfacePanel(allocator);
// Will not update if aspect ratio of parent changes
GUI::properties(panel).scaleType = GUI::ABSOLUTE;
GUI::properties(panel).dimensions = {0.2 * windowPanel.width(), windowPanel.height()};
// Padding from window
GUI::properties(panel).positionType = GUI::RELATIVE;
GUI::properties(panel).position = {0.01, 0.01};

// Will store a pointer to the properties of the windowPanel, and use those
// for alignment purposes, panel sticks to top left
panel.anchor(GUI::ANCHOR::LEFT | GUI::ANCHOR::TOP, windowPanel);

auto btn = GUI::Button(allocator);
// Will adjust even if aspect ratio of parent changes
GUI::Properties(btn).scaleType = GUI::RELATIVE;
GUI::Properties(btn).dimensions = GUI::Square(0.5); // Square spanning 1/2 width and ? height
btn.addListener(GUI::ON_CLICK, func);
btn.addListener(GUI::ON_CLICK, func2);

btn.anchor(GUI::ANCHOR::LEFT | GUI::ANCHOR::TOP, windowPanel);

// Draw the window panel, and draw all its children (GUI::INFINITE is default)
// Cap on recursion depth at `BIG_NUMBER` (specified in options somewhere)
GUI::drawRecursive(windowPanel, depth=GUI::INFINITE);

GUI::close(btn);
GUI::close(panel);
GUI::close(windowPanel);
```

### Elements

#### Properties
Structure to store basic information about every GUI element

##### Attributes
- `f2 dimensions`
- `vector<Properties*> children`
- `Properties* parent`
- `f2 position`
- `Scale scaleType`
- `Position positionType`
- `bool isVisible`

#### List
For an automatically resizing and spaced list of GUI elements

```c++
auto list = GUI::List(allocator);
// Will fill the panel its assigned to
GUI::Properties(list).scaleType = GUI::RELATIVE;
// List will automatically grow/shrink elements in height to fit specified dimensions
GUI::Properties(list).dimensions = {1.0, 0.0};

panel.anchor(GUI::Anchor::LEFT | GUI::Anchor::TOP, list);
list.elements = Vivium::Map([](button) {return &GUI::Properties(button); }, { button1, button2, button3 });
```

#### Button

```c++
GUI::Button(allocator);
```

#### Panel

#### Slider

#### Text

#### Text input

## Rendering

### Primitives

#### Buffer #Handle
Specify size and usage, will be allocated by static or dynamic allocator, on submit will specify memory type for buffer. 

#### Descriptor set #Handle 
Specify source of data for each uniform (e.g. buffer, texture).

#### Descriptor layout #Handle
Specify layout of uniforms - what data is in each uniform, and what slot is each uniform bound to.

### Texture #Handle 
...

### Shader #Handle 
...

### Layout #Object
## Types
### #Object

Objects are types which are trivially copyable, having an explicit create command, and explicit free. All methods must be const (since edits to one object wouldn't affect another).
#### Example
```c++
Type::Object inst = Type::create(...);
inst.method();

Type::Object copy = inst;
copy.method();

// Only release one instance, releasing both results in double-free
Type::release(inst);
```

### #ComplexObject

Complex objects are types which have an editable state, thus cannot be trivially copied, only moved. They do not require an explicit free, as this is done in the destructor.

#### Example
```c++
Type::ComplexObject inst(constructor...);
inst.method();

Type::ComplexObject moved = std::move(inst);

// UB since we moved the value of instance
inst.method();
```

### #Handle

Handles are pointers to data that should have all access carefully controlled and scrutinised (like abstractions of Vulkan objects). They must be freed, and are usually created by an allocator.

#### Example
```c++
StaticAllocator alloc;

NamespaceType::Handle handle = alloc.submit(...);
// Usage of handle is UB since handle data hasn't been created yet
alloc.allocate();
// Now usage of handle is valid
NamespaceType::method(handle);

NamespaceType::close(handle);
```

or

```c++
DynamicAllocator alloc;
NamespaceType::Handle handle = alloc.submit(...);
// Usage of handle is valid, handle is fully valid object already
NamespaceType::method(handle);
// Must use dynamic free for objects created by dynamic allocator
NamespaceType::closeDynamic(handle);
```

or

```c++
Window::Handle win = Window::create(Window::Options{});

while (Window::isOpen(win)) {
	Window::startFrame(win);
	Window::endFrame(win);
}

Window::close(win);
```
### #Resource

A sensitive piece of data that should only be accessible to user through a #Handle. Mostly for abstractions of Vulkan objects. Not exposed to user.

## Examples

Exemplar program

```c++
#include <vivium3.h>

using namespace Vivium;

int void main() {
	// Future potential to allow for linear/pool/rbtree?
	Storage::Scoped<Allocator::Linear> storage = Storage::Scoped<Allocator::Linear>();
	Engine::Handle engine = Engine::create(storage, Engine::Options{});
	Window::Handle window = Window::create(storage, Window::Options{});

	Engine::setPrimaryWindow(engine, window);

	// Renderer contains command buffer
	Renderer::Handle renderer = Renderer::create(storage);
	Renderer::setContext(engine, window);

	// Device contains logical device (at the least)
	Device::Handle device = Device::create(storage, engine);

	while (Window::isOpen(window)) {
		Graphics::Commands::drawIndexed(renderer, ...);
	}

	Window::close(window);
	Engine::close(engine);
}
```

## Consequence of implementation

### Allocators
- [ ] Objects like shaders and pipelines should be submitted to a queue in static allocator, then all executed after `StaticAllocator::allocate`
- [ ] Free tree allocator

### Engine
- [ ] Things like input update performed by user
- [ ] Framerate limit imposed by user, given access to some `Time::sleep(x seconds)`

### GUI
- [ ] A good GUI system (plan)

### Rendering pipeline
- [ ] Abstract classes like `Sprite` provided
- [ ] Separation of engine from window

### Shaders #Handle 
shader load from file or from compiled binary, better system of loading them, maybe runtime reflection if ur bothered also rewrite this nice and pretty

## Image #ComplexObject 
Image class used to load/store image files or raw pixel data, and safe destruction of that data

## Sprite #ComplexObject 
Sprite class should take an `Image` object, and create relevant #Handle s