#include "state.h"
#include "ecstest.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

void game() {
	State state;

	initialise(state);
	gameloop(state);
	terminate(state);
}

void ecs() {
	groupTest();
}

void lua_min() {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	if (luaL_dofile(L, "vivium4/res/scripts/test.lua") != LUA_OK) {
		// Get error from stack?
		VIVIUM_LOG(LogSeverity::ERROR, "Lua error: {}", lua_tostring(L, -1));

		lua_pop(L, 1);
	}
	
	lua_close(L);
}

int main(void) {
	// game();
	lua_min();

	return NULL;
}