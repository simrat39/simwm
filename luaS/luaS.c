#define _GNU_SOURCE
#include "server.h"
#include <assert.h>
#include <includes.h>
#include <lauxlib.h>
#include <lua.h>
#include <luaS/api/simwm.h>
#include <luaconf.h>
#include <lualib.h>
#include <output.h>
#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

#define SIMWM_LUA_PATH "/usr/share/simwl/lua"

void luaS_add_package_path(lua_State *L) {
  char *path;
  if (!(path = getenv("SIMWL_LUA_PATH"))) {
    path = SIMWM_LUA_PATH;
  }

  char *f_path;
  asprintf(&f_path, "package.path = package.path .. ';%s/?.lua'", path);

  luaL_dostring(L, f_path);
}

struct lua_State *luaS_init() {
  struct lua_State *L = luaL_newstate();
  server->L = L;

  luaL_openlibs(L);

  luaS_add_package_path(L);
  luaS_simwm_init();

  return L;
}

void luaS_doconfig() {
  const char *LUA_FILE = "test.lua";
  if (luaL_dofile(server->L, LUA_FILE)) {
    const char *err = lua_tostring(server->L, -1);
    wlr_log(WLR_ERROR, "%s", err);
  };
}

void luaS_fini() { lua_close(server->L); }
