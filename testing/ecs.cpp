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
    // TODO: this causes the next created entity to be big value
    // likely because of version number
    env.reg->free(env.e);

    return testPassed("Freed the entity");
  });
  endHeader(suite);

  pushHeader(suite, "Registry views");
  pushTest(suite, "Construct a view", [&env]() {
    // Construct some entities that belong to the view
    for (int i = 0; i < 10; i++) {
      Entity e;
      e = env.reg->create();

      env.reg->addComponent<float>(e, static_cast<float>(i));

      // Only even indices added to the view
      if (i % 2 == 0) {
        env.reg->addComponent<int>(e, i);
      }

      env.viewEntities.push_back(e);
    }

    // Add entity that is even but has bad float
    env.e = env.reg->create();
    env.reg->addComponent<int>(env.e, 20);
    env.viewEntities.push_back(env.e);

    env.basicView = env.reg->createView<Owned<int>, Partial<float>>();

    std::set<int> validNumbersSeen;

    // Validate that all previously created entities that should be in the view,
    // are in the view
    for (auto const& element : env.basicView) {
      VIVIUM_LOG(LogSeverity::DEBUG, "Element entity index is: {}",
                 element.entity);

      // Element that shouldn't be in the view is here
      if (static_cast<int>(element.get<float>()) % 2 != 0) {
        return testFailed(
            std::format("Entity with insufficient components in "
                        "view (partial), unexpected value {}",
                        element.get<float>()));
      }

      if (element.get<int>() == 20) {
        return testFailed(
            "Entity with insufficient components in view (owned)");
      }

      if (element.get<int>() % 2 == 0) {
        validNumbersSeen.insert(element.get<int>());
      }
    }

    if (validNumbersSeen.size() != 5) {
      return testFailed(
          "Expected 5 entities in group, but got different amount");
    }

    return testPassed("Created view successfully");
  });
  pushTest(suite, "Entities added after view are included in it", [&env]() {
    std::set<int> validNumbersSeen;

    for (int i = 10; i < 20; i += 2) {
      Entity e = env.reg->create();

      env.reg->addComponent<int>(e, i);
      env.reg->addComponent<float>(e, static_cast<float>(i));

      env.viewEntities.push_back(e);
    }

    for (auto const& element : env.basicView) {
      if (element.get<int>() % 2 == 0) {
        validNumbersSeen.insert(element.get<int>());
      }
    }

    if (validNumbersSeen.size() != 10) {
      return testFailed(
          "Expected 10 entities in group, but got different amount");
    }

    return testPassed("Entities added after creation are included");
  });
  pushTest(suite, "Destruction of view", [&env]() {
    env.reg->destroyView(env.basicView);

    return testPassed("Destroyed entity view");
  });

  endHeader(suite);

  finishSuite(suite);
  VIVIUM_LOG(LogSeverity::DEBUG, "Printed suite");

  delete env.reg;
}
}  // namespace Testing
