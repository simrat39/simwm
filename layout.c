#include "output.h"
#include "seat.h"
#include "server.h"
#include "view.h"
#include <includes.h>
#include <layout.h>
#include <luaS/api/output.h>
#include <luaS/api/view.h>
#include <luaS/api/workspace.h>

void on_new_view(struct simwm_workspace *workspace, struct simwm_view *view) {
  struct simwm_layout *layout = server->current_layout;
  lua_rawgeti(server->L, LUA_REGISTRYINDEX, layout->on_new_view);
  luaS_output_from_simwm_output(server->L, workspace->output);
  luaS_workspace_from_simwm_workspace(server->L, workspace);
  luaS_view_from_simwm_view(server->L, view);

  lua_pcall(server->L, 3, 0, 0);
}

void on_view_close(struct simwm_workspace *workspace, struct simwm_view *view) {
  struct simwm_layout *layout = server->current_layout;
  lua_rawgeti(server->L, LUA_REGISTRYINDEX, layout->on_view_close);
  luaS_output_from_simwm_output(server->L, workspace->output);
  luaS_workspace_from_simwm_workspace(server->L, workspace);
  luaS_view_from_simwm_view(server->L, view);

  lua_pcall(server->L, 3, 0, 0);
}

void layout_arrange(struct simwm_workspace *workspace) {
  struct simwm_layout *layout = server->current_layout;
  lua_rawgeti(server->L, LUA_REGISTRYINDEX, layout->arrange);
  luaS_output_from_simwm_output(server->L, workspace->output);
  luaS_workspace_from_simwm_workspace(server->L, workspace);

  lua_pcall(server->L, 2, 0, 0);
}
