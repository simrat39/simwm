---@class Layout
---@field name string Name of the Layout
---@field private userdata lightuserdata
local Layout = {}

---Logic to arrange views in a workspae.
---@param output Output Output on which the workspace resides
---@param workspace Workspace Workspace to arrange the views in
function Layout:arrange(output, workspace)end

return Layout
