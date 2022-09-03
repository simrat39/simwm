#pragma once

#include <lua.h>
#include <seat.h>

void luaS_workspace_from_simwm_workspace(lua_State *L, struct simwm_workspace *workspace);
