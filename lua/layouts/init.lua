---@class Layout
---@field name string Name of the Layout
---@field private userdata lightuserdata
local Layout = {}

---Called when a new window is created.
---@param output Output
---@param workspace Workspace
---@param view Window
function Layout.on_new_view(output, workspace, view) end

---Called when a new view is created.
---@param output Output
---@param workspace Workspace
---@param view Window
function Layout.on_view_close(output, workspace, view) end

---Logic to arrange views in a workspae.
---@param output Output Output on which the workspace resides
---@param workspace Workspace Workspace to arrange the views in
function Layout.arrange(output, workspace) end

return Layout
