#include <includes.h>
#include <lua.h>
#include <luaS/api/output.h>
#include <output.h>
#include <server.h>
#include <wayland-util.h>

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

// Creates the global simwm table to interfacw with the api
void luaS_simwm_init() {
  lua_pushcfunction(server->L, get_outputs);
  lua_setglobal(server->L, "get_outputs");
}
