print("Hello from lua")
local entity = nil

local function vSubmit()
	print("On submit called")
	entity = vCreateEntity()
end

local function vUpdate()
	print("On update")
	print(entity)
end

return {
	vSubmit = vSubmit,
	vUpdate = vUpdate
}