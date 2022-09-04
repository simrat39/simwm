local inspect = require("inspect")

---@class DwmLayout : Layout 
local DwmLayout = {
  name = "Dwm"
}

---Logic to arrange views in a workspae.
---@param output Output Output on which the workspace resides
---@param workspace Workspace Workspace to arrange the views in
function DwmLayout.arrange(output, workspace)
    local views = workspace:get_views()
    local res = output:get_resolution()

    if #views == 1 then
      views[1]:set_pos(5, 5)
      views[1]:set_size(res.width - 10, res.height - 10)
    elseif #views > 1 then
      local master_width = res.width * 0.45
      local child_count = #views - 1

      local child_width = res.width - master_width
      local child_height = res.height / (child_count)

      for index, view in ipairs(views) do
        if index == 1 then
         view:set_pos(0, 0)
         view:set_size(master_width, res.height)
        else
         view:set_pos(master_width, child_height * (index - 2))
         view:set_size(child_width, child_height)
        end
      end
    end
end

return DwmLayout
