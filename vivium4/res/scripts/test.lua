print("Hello from lua")

local function vSubmit()
	print("On submit called")
end

local function vUpdate()
	print("On update")
end

return {
	vSubmit = vSubmit
	vUpdate = vUpdate
}