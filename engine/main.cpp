#include "ecstest.h"
#include "state.h"

void editor(std::string const& projectFile) {
  State* state = new State();

  initialise(*state, projectFile);
  gameloop(*state);
  terminate(*state);

  delete state;
}

void ecs() { groupTest(); }

int main(int argc, char** argv) {
  std::string projectFile = "minesweeper/res/project.dat";

  if (argc > 1) {
    // Expect project name/directory
    projectFile = argv[1];
  }

  editor(projectFile);

  return 0;
}
