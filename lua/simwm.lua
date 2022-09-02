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

return M
