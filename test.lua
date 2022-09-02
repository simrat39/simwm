local inspect  = require("inspect")
local simwm = require("simwm")

local outputs = simwm.get_outputs()

print(inspect(outputs))

spawn("swaybg -o \\* -i ~/Media/macos-monterey-2560x1440_897687-mm-90.jpg -m fill")

for i = 1, 10, 1 do
  add_workspace(i)

  add_keymap({"Alt"}, i, function ()
    outputs[1]:set_workspace(i)
  end)
end

add_keymap({"Alt"}, "Return", function ()
  spawn("alacritty")
end)

outputs[1]:set_workspace("1")
