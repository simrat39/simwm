---@class Layout
---@field name string Name of the Layout
---@field private userdata lightuserdata
local Layout = {}

---Called when a new window is created.
---@param output Output
---@param workspace Workspace
---@param window Window
function Layout.on_new_window(output, workspace, window) end

---Called when a new window is created.
---@param output Output
---@param workspace Workspace
---@param window Window
function Layout.on_window_close(output, workspace, window) end

---Logic to arrange views in a workspae.
---@param output Output Output on which the workspace resides
---@param workspace Workspace Workspace to arrange the views in
function Layout.arrange(output, workspace) end

return Layout
