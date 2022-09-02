#pragma once

#include <lua.h>
#include <output.h>

int get_coords(lua_State *L);

void luaS_output_from_simwm_output(lua_State *L, struct simwm_output *output);
