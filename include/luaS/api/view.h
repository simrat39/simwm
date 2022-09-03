#pragma once

#include "lua.h"
#include <view.h>

void luaS_view_from_simwm_view(lua_State *L, struct simwm_view *view);
