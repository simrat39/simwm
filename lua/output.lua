---@class Output
---@field name string Name of the output
---@field description string Description of the output
---@field get_coords function Gets the xy coordinates of the output
---@field private userdata lightuserdata
local Output = {}

---Set the current workspace for the output.
---@param workspace string|number Name of the workspace
function Output:set_workspace(workspace)end

---Gets the xy coordinates of the output
---@return table
function Output:get_coords()end

---Gets the width and height of the output 
---@return table
function Output:get_resolution()end

return Output
