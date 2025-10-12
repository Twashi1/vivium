## Plan

How do we organise testing and create effective test suites?
- We want to be able to name "sections"
- We want to be able to create testing environments
    - Any individual test should be named
- Tests should be easy and fast to create
- We also want to be able to reuse a created environment or context, as well as dynamically recreating that environment as needed

What domains are we looking to test?
- ECS functionality
- Vector functionality and maths
- Physics AABB checks, intersection
- Language parsing (later, still too early in development)
- Serialisation

How do we setup, validate, and present results?

This is just a first draft of a potential system
look into
- jest
- rust
- other testing frameworks...

Task list
- should provide many versatile boolean comparisons for checking if values are approximately, etc.

```c++
// TODO: output format probably a pretty big function with lots of defines
// need formats for many different things like
// - suite headers
// - suite result
// - suite format
// - suite finish
TestSuite ecsSuite = createTestSuite("ECS", outputFormat, outputStream);

TestResult testEntityCreation(TestSuite& suite, EcsEnvironment& env) {
    Entity e = env.registry.create();

    if (e == ENTITY_NULL) {
        return testFailed("Returned null entity");
    }

    return testPassed();
}

suiteHeader("Running basic entity tests");
suiteResult(ecsSuite, "Entity creation", testEntityCreation(ecsSuite, env));
suiteResult(ecsSuite, "Entity add components", ...);
suiteResult(ecsSuite, "Entity free", ...);
suiteFormat("Additional formatting/printing");

// Optional end of results
suiteHeader("End of results");

// Print-out of suite results
suiteFinish(ecsSuite);

dropTestSuite(ecsSuite);
```

```c++
void* createECSEnvironment() { ... };
void freeECSEnvironment(void*) { ... };
bool testEntityCreation(void*);

// Provide good defaults for output format and stream
TestSuite ecsSuite = createTestSuite("ECS", outputFormat, outputStream);
TestEnvironment entity = createTestEnvironment(ecsSuite, "Entity", constructECSEnvironment, freeECSEnvironment);

// "Reset" causes the environment to be recreated
addTest(ecsSuite, "Entities can be created", entity, testEntityCreation, reset=false);
addTest(ecsSuite, "Entities can have components added", entity, testEntityAddComponent, reset=false);
// Prints a bunch of information, can we get an arbitrary output
bool passed = runSuite(ecsSuite);

dropTestEnvironment(entity);
dropTestSuite(ecsSuite);
```
