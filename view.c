#include <includes.h>

#include <server.h>
#include <view.h>

struct simwm_view *view_at(double lx, double ly, struct wlr_surface **surface,
                           double *sx, double *sy) {
  struct wlr_scene_node *node =
      wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy);

  if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
    return NULL;
  }

  struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
  struct wlr_scene_surface *scene_surface =
      wlr_scene_surface_from_buffer(scene_buffer);

  if (!scene_surface) {
    return NULL;
  }

  *surface = scene_surface->surface;

  struct wlr_scene_tree *tree = node->parent;

  while (tree != NULL && tree->node.data == NULL) {
    tree = tree->node.parent;
  }

  if (tree == NULL) {
    return NULL;
  }

  return tree->node.data;
}
