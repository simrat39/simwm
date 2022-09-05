---@class Workspace
---@field name string Name of the workspace
---@field private userdata lightuserdata
local Workspace = {}

---Gets all the views in the workspace.
---@return any[]
function Workspace:get_views()end

---Gets the currently focused window on the workspace
---@return Window
function Workspace:get_focused_window()end

return Workspace
