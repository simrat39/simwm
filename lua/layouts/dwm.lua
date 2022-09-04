---@class DwmLayout : Layout
local M = {
  name = "Dwm",
  master = {},
  stack = {},
  width = 0.5,
}

---Get the stack for a workspace
---@param ws any
---@return Window[]
local function get_stack(ws)
 return M.stack[ws.name]
end

---Sets the stack for a workspace.
---@param ws Workspace
---@param new Window[]
local function set_stack(ws, new)
 M.stack[ws.name] = new;
end

---Gets the master for a workspace.
---@param ws Workspace
local function get_master(ws)
 return M.master[ws.name];
end

---Sets the master for a workspace.
---@param ws Workspace
---@param new any
local function set_master(ws, new)
 M.master[ws.name] = new;
end

---Used to maximize the window when we only have one, which is the master.
---@param o Output
---@param ws Workspace
local function set_master_fullscreen(o, ws)
  local res = o:get_resolution()
  get_master(ws):set_pos(5, 5)
  get_master(ws):set_size(res.width - 10, res.height - 10)
end

---Called when we get atleast one view in the stack, so the master needs to be
---smaller now.
---@param o Output
---@param ws Workspace
local function set_master_pos_width(o, ws)
  local res = o:get_resolution()
  get_master(ws):set_pos(0, 0)
  get_master(ws):set_size(res.width * M.width, res.height)
end

---Sets the stack position and width
---@param o Output
---@param ws Workspace
local function set_stack_pos_size(o, ws)
  local res = o:get_resolution()
  local master_width = res.width * M.width
  local stack_count = #get_stack(ws)

  local stack_width = res.width - master_width
  local stack_height = res.height / (stack_count)

  for index, view in ipairs(get_stack(ws)) do
    view:set_pos(master_width, stack_height * (index - 1))
    view:set_size(stack_width, stack_height)
  end
end

---Called when a new view is opened
---@param o Output
---@param ws Workspace
---@param win Window
function M.on_new_window(o, ws, win)
  if not get_stack(ws) then
    set_stack(ws, {})
  end

  if get_master(ws) == nil then
    set_master(ws, win);
    set_master_fullscreen(o, ws);
    return;
  end

  table.insert(get_stack(ws), win)
  set_stack_pos_size(o, ws);

  if #get_stack(ws) >= 1 then
    set_master_pos_width(o, ws);
  end
end

---Called when a new view is opened
---@param o Output
---@param ws Workspace
---@param win Window
function M.on_window_close(o, ws, win)
  -- master was removed
  if win.userdata == get_master(ws).userdata then
    -- check if we have any views in the stack
    if #get_stack(ws) >= 1 then
      -- Set a new master if we do
      set_master(ws, get_stack(ws)[1])
      table.remove(get_stack(ws), 1);

      -- after new master is set, do we have any views in the stack
      if #get_stack(ws) >= 1 then
        -- if we do, setup the layout normally
        set_master_pos_width(o, ws);
        set_stack_pos_size(o, ws);
      else
        -- if we don't, then fullscreen current master
        set_master_fullscreen(o, ws);
      end
    -- we don;t have any candidates for the new master, so do nothihng
    else
      set_master(ws, nil)
    end
  -- view from stack was removed
  else
    for i, v in ipairs(get_stack(ws)) do
      if v.userdata == win.userdata then
        table.remove(get_stack(ws), i);
      end
    end

    -- stack was emptied
    if #get_stack(ws) == 0 then
      set_master_fullscreen(o, ws);
    else
      set_stack_pos_size(o, ws);
    end
  end
end

---Logic to arrange views in a workspae.
---@param o Output Output on which the workspace resides
---@param ws Workspace Workspace to arrange the views in
function M.arrange(o, ws) end

return M
