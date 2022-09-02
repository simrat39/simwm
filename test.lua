local inspect = require("inspect")

print("hiu");
-- print(inspect(get_outputs()))


local outputs = get_outputs()

spawn("swaybg -o \\* -i ~/Media/macos-monterey-2560x1440_897687-mm-90.jpg -m fill")

for i = 1, 10, 1 do
  add_workspace(tostring(i))

  add_keymap({"Alt"}, tostring(i), function ()
    outputs[1]:set_workspace(tostring(i))
  end)
end

add_keymap({"Alt"}, "Return", function ()
  spawn("alacritty")
end)

outputs[1]:set_workspace("1")
