local sim = require("simwm")
local inspect = require("inspect")

local outputs = sim.get_outputs()

sim.spawn("swaybg -o \\* -i ~/Media/macos-monterey-2560x1440_897687-mm-90.jpg -m fill")

for i = 1, 10, 1 do
  sim.add_workspace(i)

  sim.map({ "Alt" }, i, function()
    outputs[1]:set_workspace(i)
  end)
end

sim.map({ "Alt" }, "Return", function()
  sim.spawn("alacritty")
end)

outputs[1]:set_workspace("1")

sim.register_layout_manager({
  name = "DWL",
  arrange = function(o, w)
    local views = w:get_views()
    local res = o:get_resolution()

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
})
