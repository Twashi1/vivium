## Plan

How do we organise testing and create effective test suites?
- We want to be able to name "sections"
- We want to be able to create testing environments
    - Any individual test should be named
- Tests should be easy and fast to create

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
