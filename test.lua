local sim = require("simwm")
local dwm = require("layouts/dwm")

local outputs = sim.get_outputs()

sim.spawn("swaybg -o \\* -i ~/Media/macos-monterey-2560x1440_897687-mm-90.jpg -m fill")

for i = 1, 10, 1 do
	sim.add_workspace(i)

	sim.map({ "Alt" }, i, function()
		outputs[1]:set_workspace(i)
	end)
end

sim.map({ "Alt" }, "Return", function()
	sim.spawn("kitty")
end)

sim.map({ "Alt" }, "w", function()
	local o = sim.get_focused_output()
	local ws = o:get_current_workspace()
	local win = ws:get_focused_window()

	win:close()
end)

sim.map({ "Alt" }, "l", function()
	dwm.update_ratio(dwm.width + 0.1)
end)

sim.map({ "Alt" }, "h", function()
	dwm.update_ratio(dwm.width - 0.1)
end)

outputs[1]:set_workspace("1")

sim.register_layout_manager(dwm)
