## Errors

Non-fatal

For fatal return values, just early return with code `V4_RESULT_FATAL_xxx` (use bitmask for easy check)

```c++
auto returnValue;

Result errorCode = dangerousFunction(&returnValue);

if (errorCode == FILE_NOT_FOUND)
	doSomething;
```
## Concurrency

Should be explicit about what is thread-safe and what isn't, but onus is on user to build concurrent systems. Maybe you can write a wrapper for a simple `for` loop.
## Time

### Interval

Execute a function at a given interval. Larger time intervals can use some sort of thread sleep, shorter ones may need a `nanosleep`. Will require `nanosleep` to be thread-safe.

```c++
Time::Interval command = Time::Interval(5ms, myFunction, callee, userArgs);

// Starts the thread
Time::start(command);

// Async statements run normally
while (gameloop) {}

// Stops the thread (thread.join)
Time::stop(command);
```

## Examples

Exemplar program

```c++
#include <vivium4.h>

using namespace Vivium;

int void main() {
	Storage::Static::Pool pool = Storage::createPool(0x1000);

	Engine engine = createEngine(pool, EngineOptions{ ... });
	Window primary = createWindow(pool, engine, WindowOptions{ ... });

	Allocator allocator = createAllocator(pool, engine);

	beginFrame(engine, primary);
	beginRender(primary);
	
	endRender(primary);
	endFrame(engine, primary);

	freeEngine(engine);

	return NULL;
}
```