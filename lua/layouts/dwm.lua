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

---Called when a new view is opened
---@param output Output
---@param workspace Workspace
---@param view any
function M.on_view_close(output, workspace, view)
  -- master was removed
  if view.userdata == M.master.userdata then
    -- check if we have any views in the stack
    if #M.stack >= 1 then
      -- Set a new master if we do
      M.master = M.stack[1]
      table.remove(M.stack, 1);

      -- after new master is set, do we have any views in the stack
      if #M.stack >= 1 then
        -- if we do, setup the layout normally
        set_master_pos_width(output);
        set_stack(output);
      else
        -- if we don't, then fullscreen current master
        set_master_fullscreen(output);
      end
    -- we don;t have any candidates for the new master, so do nothihng
    else
      M.master = nil
    end
  -- view from stack was removed
  else
    for i, v in ipairs(M.stack) do
      if v.userdata == view.userdata then
        table.remove(M.stack, i);
      end
    end

    -- stack was emptied
    if #M.stack == 0 then
      set_master_fullscreen(output);
    else
      set_stack(output);
    end
  end
end

---Logic to arrange views in a workspae.
---@param output Output Output on which the workspace resides
---@param workspace Workspace Workspace to arrange the views in
function M.arrange(output, workspace) end

return M
