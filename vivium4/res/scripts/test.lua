print("Hello from lua")
local entity0 = nil
local entity1 = nil
local entity2 = nil
local entity3 = nil
local entity4 = nil

--[[
TODO: redo this it changed
Entity 0 - Buffer layout / Descriptor layout
Entity 1 - Vertex buffer / Vertex shader / Descriptor
Entity 2 - Index buffer / Fragment shader
Entity 3 - Pipeline
]]

local function vSubmit()
	print("On submit called")
	entity0 = vGetEntityByID(0)
	entity1 = vGetEntityByID(1)
	entity2 = vGetEntityByID(2)
	entity3 = vGetEntityByID(3)
	entity4 = vGetEntityByID(4)

	vSetBufferData({
		0.0,	0.0,	1.0, 0.0, 0.0,
		200.0,	0.0,	0.0, 1.0, 0.0,
		200.0,	200.0,	0.0, 0.0, 1.0,
		0.0,	200.0,  1.0, 0.0, 1.0
	}, vFLOAT, entity2)

	vSetBufferData({
		0, 1, 2, 2, 3, 0
	}, vUINT16, entity3)

	vSetBufferData({
		0.0
	}, vFLOAT, entity4)
end

local function vUpdate()
	print("On update")
end

return {
	vSubmit = vSubmit,
	vUpdate = vUpdate
}