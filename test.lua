local inspect = require("inspect")

print("hiu");
-- print(inspect(get_outputs()))


for i = 1, 10, 1 do
  add_workspace(tostring(i))
end

local outputs = get_outputs()

for index, value in ipairs(outputs) do
 print(inspect(value))
end

outputs[1]:set_workspace("1")

