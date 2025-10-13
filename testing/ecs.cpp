#include "ecs.h"

namespace Testing {
void runECSTest() {
  ECSEnv env;
  env.reg = new Registry();

  VIVIUM_LOG(LogSeverity::DEBUG, "Created regsitry");

  TestSuite suite =
      createSuite("ECS suite", defaultFormatter(), consoleOutstream);
  pushHeader(suite, "Basic create and drop entity");
  pushResult(suite, "Create an entity", [&env]() {
    Entity entity = env.reg->create();
    return entity == ECS_ENTITY_DEAD ? testFailed("Returned null entity")
                                     : testPassed("");
  }());
  pushResult(suite, "Dummy test", testPassed("Dummy passed"));
  endHeader(suite);
  finishSuite(suite);
  VIVIUM_LOG(LogSeverity::DEBUG, "Printed suite");

  delete env.reg;
}
}  // namespace Testing
