---@class Window
---@field app_id string Name of the window
---@field private userdata lightuserdata
local Window = {}

---Set the current workspace for the output.
---@param x number x coordinate globally
---@param y number y coordinate globally
function Window:set_pos(x, y)end

---Set the size of the window
---@param width number
---@param height number
function Window:set_size(width, height)end

---Close the window.
function Window:close()end

return Window
