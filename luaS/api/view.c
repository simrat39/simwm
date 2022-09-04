#include "view.h"
#include "lua.h"
#include <includes.h>
#include <luaS/utils/dump.h>
#include <output.h>
#include <wlr/util/log.h>

int luaS_view_set_size(lua_State *L) {
  lua_getfield(L, 1, "userdata");
  struct simwm_view *view = lua_touserdata(L, -1);

  lua_Number width = lua_tonumber(L, 2);
  lua_Number height = lua_tonumber(L, 3);

  switch (view->type) {
  case SIMWM_VIEW_XDG:
    wlr_xdg_toplevel_set_resizing(view->xdg->toplevel, true);
    wlr_xdg_toplevel_set_size(view->xdg->toplevel, width, height);
    wlr_xdg_toplevel_set_resizing(view->xdg->toplevel, false);
    break;
  case SIMWM_VIEW_LAYER:
  case SIMWM_VIEW_POPUP:
    break;
  }
  return 0;
}

int luaS_view_set_pos(lua_State *L) {
  lua_getfield(L, 1, "userdata");
  struct simwm_view *view = lua_touserdata(L, -1);

  lua_Number x = lua_tonumber(L, 2);
  lua_Number y = lua_tonumber(L, 3);

  switch (view->type) {
  case SIMWM_VIEW_XDG:
    wlr_scene_node_set_position(&view->xdg->scene->node, x, y);
    break;
  case SIMWM_VIEW_LAYER:
  case SIMWM_VIEW_POPUP:
    break;
  }
  return 0;
}

// Sets up a simwm_view table in lua and puts it on top of the stack.
void luaS_view_from_simwm_view(lua_State *L, struct simwm_view *view) {
  lua_newtable(L);

  lua_pushstring(L, "name");
  lua_pushnumber(L, view->type);
  lua_settable(L, -3);

  lua_pushstring(L, "set_pos");
  lua_pushcfunction(L, luaS_view_set_pos);
  lua_settable(L, -3);

  lua_pushstring(L, "set_size");
  lua_pushcfunction(L, luaS_view_set_size);
  lua_settable(L, -3);

  lua_pushstring(L, "userdata");
  lua_pushlightuserdata(L, view);
  lua_settable(L, -3);
}
