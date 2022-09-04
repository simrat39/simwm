#pragma once

#include <lua.h>

#include <view.h>
#include <xdg_shell.h>

void luaS_window_from_simwm_xdg_surface(lua_State *L,
                                        struct simwm_xdg_surface *window);
