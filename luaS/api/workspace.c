#include "lua.h"
#include <includes.h>
#include <seat.h>

// Sets up a simwm_output table in lua and puts it on top of the stack.
void luaS_workspace_from_simwm_workspace(lua_State *L,
                                         struct simwm_workspace *workspace) {
  lua_newtable(L);

  lua_pushstring(L, "name");
  lua_pushstring(L, workspace->name);
  lua_settable(L, -3);
}
