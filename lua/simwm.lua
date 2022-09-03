---@diagnostic disable: undefined-global
local M = {}

---Get the current availaible outputs.
---@return Output[]
function M.get_outputs()
  return get_outputs()
end

---Runs a command through /bin/sh
---@param command string
function M.spawn(command)
  return spawn(command)
end

---Set a keybinding.
---@param modifiers table
---@param key string|number
---@param on_press function
---@param on_release? function
function M.map(modifiers, key, on_press, on_release)
  add_keymap(modifiers, key, on_press, on_release)
end

---Creates a new workspace.
---@param name string|number
function M.add_workspace(name)
  add_workspace(name)
end

---Registers a layout manager.
---@param manager Layout
function M.register_layout_manager(manager)
  register_layout_manager(manager)
end

return M
