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
auto windowPanel = GUI::Panel();
// Will make window span the entire screen (SPECIAL!)
windowPanel.setParent(Window::getPanel(window));

auto panel = GUI::InterfacePanel(allocator);
panel.name = "Buttons";
// Will not update if aspect ratio of parent changes
panel.scaleType = GUI::ABSOLUTE;
panel.dimensions = {0.2 * windowPanel.width(), windowPanel.height()};
// Padding from window
panel.positionType = GUI::RELATIVE;
panel.position = {0.01, 0.01};

panel.clearParent();
panel.setParent(nullptr);

windowPanel.addChild(panel);

// Panel will stick to top left
// Will throw error if panel has a parent
windowPanel.anchor(GUI::ANCHOR::LEFT | GUI::ANCHOR::TOP, panel);

auto btn = GUI::Button(allocator);
btn.scaleType = GUI::RELATIVE;
// Will adjust even if aspect ratio of parent changes
btn.dimensions = GUI::Square(0.5); // Square spanning 1/2 width and ? height
btn.addEvent(GUI::ON_CLICK, func);
btn.addEvent(GUI::ON_CLICK, func2);

windowPanel.addChild(btn);

// Draw the window panel, and draw all its children (GUI::INFINITE is default)
// Cap on recursion depth at `BIG_NUMBER` (specified in options somewhere)
GUI::drawRecursive(windowPanel, depth=GUI::INFINITE);

GUI::close(btn);
GUI::close(panel);
GUI::close(windowPanel);
```

### Elements

#### Base
Base class of all GUI objects

##### Properties
- `vec2 dimensions`
- `vector<Base*> children`
- `Base* parent`
- `vec2 position`
- `GUI::SCALE_TYPE scaleType`
- `GUI::POSITION_TYPE positionType`
- `bool isVisible`

```c++

```

#### List
For an automatically resizing list of GUI objects

```c++
auto list = GUI::List(allocator);
list.scaleType = GUI::RELATIVE;
// List will automatically grow/shrink in height to contain its elements
list.dimensions = {1.0, 0.0};

// Equivelant to addChild
list.setParent(panel);
// TODO: appendChild, or pass vector of GUI objects (as void*, as polymorphic elements)?
list.appendChild(button1, button2, button3);
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
	Engine::Handle engine = Engine::create(Engine::Options{});
	Window::Handle window = Window::create(Window::Options{});

	Engine::setWindow(engine, window);

	while (Window::isOpen(window)) {
		Graphics::Commands::drawIndexed(engine, ...);
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