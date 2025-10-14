#include "ecs.h"

namespace Testing {
void runECSTest() {
  ECSEnv env;
  env.reg = new Registry();

  VIVIUM_LOG(LogSeverity::DEBUG, "Created regsitry");

  TestSuite suite =
      createSuite("ECS suite", defaultFormatter(), consoleOutstream);
  pushHeader(suite, "Basic entity operations");
  pushTest(suite, "Create an entity", [&env]() {
    env.e = env.reg->create();
    return env.e == ECS_ENTITY_DEAD ? testFailed("Returned null entity")
                                    : testPassed("");
  });
  pushTest(suite, "Register a component type", [&env]() {
    env.reg->registerComponent<int>();

    return testPassed("Registered integer component");
  });
  pushTest(suite, "Add a POD component to an entity", [&env]() {
    env.reg->addComponent(env.e, 500);

    return testPassed("Added component to entity");
  });
  pushTest(suite, "Read out the component", [&env]() {
    int readOut = env.reg->getComponent<int>(env.e);

    return readOut == 500 ? testPassed("Correctly read value")
                          : testFailed("Invalid value readout");
  });
  pushTest(suite, "Test removal of the component", [&env]() {
    env.reg->removeComponent<int>(env.e);

    if (env.reg->hasComponent<int>(env.e)) {
      return testFailed("Entity signature still contained the component");
    }

    return testPassed("Removed the component");
  });
  pushTest(suite, "Delete the entity", [&env]() {
    env.reg->free(env.e);

    return testPassed("Freed the entity");
  });
  endHeader(suite);
  finishSuite(suite);
  VIVIUM_LOG(LogSeverity::DEBUG, "Printed suite");

  delete env.reg;
}
}  // namespace Testing
