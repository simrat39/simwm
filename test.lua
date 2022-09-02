local sim = require("simwm")

local outputs = sim.get_outputs()

sim.spawn("swaybg -o \\* -i ~/Media/macos-monterey-2560x1440_897687-mm-90.jpg -m fill")

for i = 1, 10, 1 do
  sim.add_workspace(i)

  sim.map({"Alt"}, i, function ()
    outputs[1]:set_workspace(i)
  end)
end

sim.map({"Alt"}, "Return", function ()
  sim.spawn("alacritty")
end)

outputs[1]:set_workspace("1")
