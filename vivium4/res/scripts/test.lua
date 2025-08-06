print("Hello from lua")
local entity0 = nil
local entity1 = nil
local entity2 = nil
local entity3 = nil
local entity4 = nil
local entity5 = nil

local storageBufferData = {}
local gridWidth = 8
local gridHeight = 8
local gridSize = gridWidth * gridHeight
local bombCount = 15
local tileGrid = {}
local renderGrid = {}

local TILE_UNREVEALED = 0
local TILE_FLAG = 1
local TILE_BOMB = 2
local TILE_ZERO = 3

local ATLAS_INDICES = {
	{3, 1}, -- 0, unrevealed
	{1, 1}, -- 1, flag
	{0, 1}, -- 2, bomb
	{2, 1}, -- 3, zero (revealed tile)
	{0, 0}, -- 3 + 1, number tile
	{1, 0}, -- 3 + 2, number tile
	{2, 0}, -- 3 + 3, number tile
	{3, 0}, -- 3 + 4, number tile
	{4, 0}, -- 3 + 5, number tile
	{5, 0}, -- 3 + 6, number tile
	{6, 0}, -- 3 + 7, number tile
	{7, 0}  -- 3 + 8, number tile
}

local ATLAS_SIZE_PX = {128, 32}
local SPRITE_SIZE_PX = {16, 16}
local TILE_SIZE_PX = {32, 32}

local atlasConverted = {}

local X_COORD = 1
local Y_COORD = 2

-- TODO: ensure everything is 1 indexed correctly, even with 2D arrays, use helper functions

--[[
TODO: redo this it changed
Entity 0 - Pipeline
Entity 1 - Texture, BufferLayout (VEC2, VEC2), DescriptorLayout (Vertex Storage, Fragment Texture)
Entity 2 - Vertex buffer, Vertex shader
Entity 3 - Index buffer, Fragment shader
Entity 4 - Storage buffer,
Entity 5 - Descriptor Set (4, 1)
]]

-- Given 2D 1-based index, return 1D 1-based index
local function index2Dto1D(index)
	return index[X_COORD] + (index[Y_COORD] - 1) * gridWidth
end

local function multiplyScalar(s, v)
	return { (s * v[X_COORD]), (s * v[Y_COORD]) }
end

local function multiplyPairwise(v1, v2)
	return { v1[X_COORD] * v2[X_COORD], v1[Y_COORD] * v2[Y_COORD] }
end

local function atlasIndex(atlasSizePx, spriteSizePx, atlasIndex)
	local inverseWidth = 1.0 / atlasSizePx[X_COORD]
	local inverseHeight = 1.0 / atlasSizePx[Y_COORD]

	-- Note we convert to 0 index here
	local atlasLeft = atlasIndex[X_COORD] - 1
	local atlasRight = atlasIndex[X_COORD]
	local atlasBot = atlasIndex[Y_COORD] - 1
	local atlasTop = atlasIndex[Y_COORD]

	local coordLeft = atlasLeft * inverseWidth * spriteSizePx[X_COORD]
	local coordRight = atlasRight * inverseWidth * spriteSizePx[X_COORD]
	local coordBot = atlasTop * inverseHeight * spriteSizePx[Y_COORD]
	local coordTop = atlasBot * inverseHeight * spriteSizePx[Y_COORD]

	local translation = { coordLeft, coordTop }
	local scale = { (coordRight - coordLeft), (coordBot - coordTop) }

	return {
		left = coordLeft,
		right = coordRight,
		bottom = coordBot,
		top = coordTop,
		translation = translation,
		scale = scale
	}
end

local function generateAllAtlas()
	for i, val in ipairs(ATLAS_INDICES) do
		-- Note will start at 1
		atlasConverted[i] = atlasIndex(ATLAS_SIZE_PX, SPRITE_SIZE_PX, val)
	end
end

local function createGrid()
	for i = 1, gridSize do
		tileGrid[i] = TILE_UNREVEALED
		renderGrid[i] = TILE_UNREVEALED
	end
end

local function populateBombs(clickPosition1D)
	local bombsRemaining = bombCount

	for i = 1, gridSize do
		tileGrid[i] = TILE_ZERO
		
		if math.random(0, gridSize - i) < bombsRemaining then
			if i ~= clickPosition1D then
				bombsRemaining = bombsRemaining - 1

				tileGrid[i] = TILE_BOMB
			end
		end
	end

	-- Fill in the numbers
	for y = 1, gridHeight do
		for x = 1, gridWidth do
			local numBombs = 0

			for dy = -1, 1 do
				local ny = y + dy

				for dx = -1, 1 do
					local nx = x + dx

					if tileGrid[index2Dto1D({nx, ny})] == TILE_BOMB then
						numBombs = numBombs + 1
					end
				end
			end

			if tileGrid[index2Dto1D({x, y})] ~= TILE_BOMB then
				tileGrid[index2Dto1D({x, y})] = TILE_ZERO + numBombs
			end
		end
	end
end

local function populateStorageBuffer()
	local storageIndex = 1
	
	for y = 1, gridHeight do
		for x = 1, gridWidth do
			local tileTranslation = multiplyPairwise({x - 1, y - 1}, TILE_SIZE_PX)
			local tileScale = TILE_SIZE_PX

			-- TODO: use render grid in future, not tile grid
			local tileType = tileGrid[index2Dto1D({x, y})]
			-- Tile types are 0-indexed, convert to 1-indexed
			local tileData = atlasConverted[tileType + 1]

			local textureTranslation = tileData.translation
			local textureScale = tileData.scale

			storageBufferData[storageIndex + 0] = tileTranslation[X_COORD]
			storageBufferData[storageIndex + 1] = tileTranslation[Y_COORD]
			print(storageBufferData[storageIndex], storageBufferData[storageIndex + 1])

			storageBufferData[storageIndex + 2] = textureTranslation[X_COORD]
			storageBufferData[storageIndex + 3] = textureTranslation[Y_COORD]

			storageBufferData[storageIndex + 4] = textureScale[X_COORD]
			storageBufferData[storageIndex + 5] = textureScale[Y_COORD]

			storageBufferData[storageIndex + 6] = tileScale[X_COORD]
			storageBufferData[storageIndex + 7] = tileScale[Y_COORD]

			storageIndex = storageIndex + 8
		end
	end
end

local function vSubmit()
	generateAllAtlas()
	createGrid()
	populateBombs()

	print("On submit called")
	entity0 = vGetEntityByID(0)
	entity1 = vGetEntityByID(1)
	entity2 = vGetEntityByID(2)
	entity3 = vGetEntityByID(3)
	entity4 = vGetEntityByID(4)
	entity5 = vGetEntityByID(5)

	vSetBufferData({
		0.0,	0.0,  0.0, 0.0,
		1.0,	0.0,  0.0, 1.0,
		1.0,	1.0,  1.0, 1.0,
		0.0,	1.0,  1.0, 0.0
	}, vFLOAT, entity2)

	vSetBufferData({
		0, 1, 2, 2, 3, 0
	}, vUINT16, entity3)

	for i = 1, gridSize * 8 do
		storageBufferData[i] = 0.0
	end

	vSetBufferData(storageBufferData, vFLOAT, entity4)
end

local function vUpdate()
	-- TODO: shouldn't do every frame, jsut when theres a change
	populateStorageBuffer()
	vSetBufferData(storageBufferData, vFLOAT, entity4)
end

local function vDraw()
	vDrawIndex(entity0, gridSize)
end

return {
	vSubmit = vSubmit,
	vUpdate = vUpdate,
	vDraw = vDraw
}