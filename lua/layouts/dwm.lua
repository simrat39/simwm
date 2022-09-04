---@class DwmLayout : Layout
local M = {
  name = "Dwm",
  master = nil,
  stack = {},
  width = 0.5,
}

---Used to maximize the window when we only have one, which is the master.
---@param output Output
local function set_master_fullscreen(output)
  local res = output:get_resolution()
  M.master:set_pos(5, 5)
  M.master:set_size(res.width - 10, res.height - 10)
end

---Called when we get atleast one view in the stack, so the master needs to be
---smaller now.
---@param output Output
local function set_master_pos_width(output)
  local res = output:get_resolution()
  M.master:set_pos(0, 0)
  M.master:set_size(res.width * M.width, res.height)
end

---Sets the stack position and width
---@param output Output
local function set_stack(output)
  local res = output:get_resolution()
  local master_width = res.width * M.width
  local stack_count = #M.stack

  local stack_width = res.width - master_width
  local stack_height = res.height / (stack_count)

  for index, view in ipairs(M.stack) do
    view:set_pos(master_width, stack_height * (index - 1))
    view:set_size(stack_width, stack_height)
  end
end

---Called when a new view is opened
---@param output Output
---@param workspace Workspace
---@param view any
function M.on_new_view(output, workspace, view)
  if M.master == nil then
    M.master = view;
    set_master_fullscreen(output);
    return;
  end

  if #M.stack then
    set_master_pos_width(output);
  end

  table.insert(M.stack, view)
  set_stack(output);
end

---Logic to arrange views in a workspae.
---@param output Output Output on which the workspace resides
---@param workspace Workspace Workspace to arrange the views in
function M.arrange(output, workspace)
  local views = workspace:get_views()
  local res = output:get_resolution()

  if #views == 1 then
  elseif #views > 1 then
    local master_width = res.width * 0.45
    local child_count = #views - 1

    local child_width = res.width - master_width
    local child_height = res.height / (child_count)

    for index, view in ipairs(views) do
      if index == 1 then
        view:set_pos(0, 0)
        view:set_size(master_width, res.height)
      else
        view:set_pos(master_width, child_height * (index - 2))
        view:set_size(child_width, child_height)
      end
    end
  end
end

return M
