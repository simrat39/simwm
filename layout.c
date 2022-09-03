#include "output.h"
#include "server.h"
#include <includes.h>
#include <layout.h>
#include <luaS/api/output.h>
#include <luaS/api/workspace.h>

void layout_arrange(struct simwm_workspace *workspace) {
  struct simwm_layout *layout = server->current_layout;
  lua_rawgeti(server->L, LUA_REGISTRYINDEX, layout->arrange);
  luaS_output_from_simwm_output(server->L, workspace->output);
  luaS_workspace_from_simwm_workspace(server->L, workspace);

  lua_pcall(server->L, 2, 0, 0);
}
