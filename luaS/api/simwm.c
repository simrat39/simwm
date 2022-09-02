#include <includes.h>
#include <lua.h>
#include <luaS/api/output.h>
#include <output.h>
#include <seat.h>
#include <server.h>
#include <string.h>
#include <wlr/util/log.h>

int get_outputs(lua_State *L) {
  lua_newtable(L);

  struct simwm_output *output;
  int idx = 1;

  wl_list_for_each(output, &server->outputs, link) {
    // Make output object and put it on top of the stack
    luaS_output_from_simwm_output(L, output);

    // Table is now at -2 in the stack, pop the current element and add it to
    // the table.
    lua_rawseti(L, -2, idx);
    idx++;
  }

  return 1;
}

int add_workspace(lua_State *L) {
  wlr_log(WLR_INFO, "ADDING WORKSPACE");

  if (!lua_isstring(L, -1)) {
    wlr_log(WLR_ERROR, "Workspace name needs to be a string");
  }
  const char *ws_name = lua_tostring(L, -1);

  // Lua GC might free the string above, so copy it.
  seat_add_workspace(strdup(ws_name));
  return 0;
}

// Creates the global simwm table to interfacw with the api
void luaS_simwm_init() {
  lua_pushcfunction(server->L, get_outputs);
  lua_setglobal(server->L, "get_outputs");

  lua_pushcfunction(server->L, add_workspace);
  lua_setglobal(server->L, "add_workspace");
}
