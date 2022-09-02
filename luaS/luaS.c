#include "server.h"
#include <assert.h>
#include <includes.h>
#include <lauxlib.h>
#include <lua.h>
#include <luaS/api/simwm.h>
#include <lualib.h>
#include <output.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

struct lua_State *luaS_init() {
  struct lua_State *L = luaL_newstate();
  server->L = L;

  luaL_openlibs(L);

  luaS_simwm_init();

  return L;
}

void luaS_doconfig() {
  const char *LUA_FILE = "test.lua";
  luaL_dofile(server->L, LUA_FILE);
}

void luaS_fini() { lua_close(server->L); }
