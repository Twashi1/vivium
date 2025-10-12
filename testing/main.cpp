#include "ecs.h"

int main() {
  _logInit();
  VIVIUM_LOG(LogSeverity::DEBUG, "Running ECS test");
  Testing::runECSTest();
  VIVIUM_LOG(LogSeverity::DEBUG, "Finished all tests");

  return 0;
}
