print("Hello from lua")

--[[
on first click, the area we reveal must be an empty space
given the click position, we must not allow a bomb in the surrounding 8 squares

]]

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

local hasRevealedBoard = false

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

local function invert(v)
	return { 1.0 / v[X_COORD], 1.0 / v[Y_COORD] }
end

local function applyPairwise(v, f)
	return { f(v[X_COORD]), f(v[Y_COORD]) }
end

local function inspect(table)
	local string = "{ "
	for i, val in ipairs(table) do
		string = string .. tostring(i) .. ": " .. tostring(val) .. ","
	end
	
	return string .. "}"
end

local function atlasIndex(atlasSizePx, spriteSizePx, atlasIndex)
	local inverseWidth = 1.0 / atlasSizePx[X_COORD]
	local inverseHeight = 1.0 / atlasSizePx[Y_COORD]

	-- Note we convert to 0 index here
	local atlasLeft = atlasIndex[X_COORD]
	local atlasRight = atlasIndex[X_COORD] + 1
	local atlasBot = atlasIndex[Y_COORD]
	local atlasTop = atlasIndex[Y_COORD] + 1

	local coordLeft = atlasLeft * inverseWidth * spriteSizePx[X_COORD]
	local coordRight = atlasRight * inverseWidth * spriteSizePx[X_COORD]
	local coordBot = atlasBot * inverseHeight * spriteSizePx[Y_COORD]
	local coordTop = atlasTop * inverseHeight * spriteSizePx[Y_COORD]

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

local function contains(table, value)
	for _, v in ipairs(table) do
		if v == value then
			return true
		end
	end

	return false
end

local function populateBombs(clickPosition2D)
	local bombsRemaining = bombCount

	local bannedPositions = {}
	local bannedPositionCount = 0

	-- Ensure all surrounding tiles of the click position are not bombs
	for dy = -1, 1 do
		local ny = clickPosition2D[Y_COORD] + dy

		if ny <= 0 or ny >= gridHeight then
			goto continueY
		end

		for dx = -1, 1 do
			local nx = clickPosition2D[X_COORD] + dx

			if nx <= 0 or nx >= gridWidth then
				goto continueX
			end

			bannedPositionCount = bannedPositionCount + 1
			bannedPositions[bannedPositionCount] = index2Dto1D({ nx, ny })

			::continueX::
		end

		::continueY::
	end

	for i = 1, gridSize do
		tileGrid[i] = TILE_ZERO
		
		-- TODO: messes with the randomness, can cause less bombs to spawn than normal as well
		-- to fix, generate bombs assuming the grid to have n - #bannedPositions
		-- then for any bomb s.t. index >= min(bannedPositions), move "around" the banned position square (difficult...)
		if math.random(0, gridSize - i) < bombsRemaining then
			if not contains(bannedPositions, i) then
				bombsRemaining = bombsRemaining - 1

				tileGrid[i] = TILE_BOMB
			end
		end
	end

	print("Before population")
	print(inspect(tileGrid))

	-- Fill in the numbers
	for y = 1, gridHeight do
		for x = 1, gridWidth do
			local numBombs = 0

			for dy = -1, 1 do
				local ny = y + dy

				if ny <= 0 or ny > gridHeight then
					goto continue
				end

				for dx = -1, 1 do
					local nx = x + dx

					if nx <= 0 or nx > gridWidth then
						goto continue2
					end

					if tileGrid[index2Dto1D({nx, ny})] == TILE_BOMB then
						numBombs = numBombs + 1
					end

					::continue2::
				end

				::continue::
			end

			if tileGrid[index2Dto1D({x, y})] ~= TILE_BOMB then
				tileGrid[index2Dto1D({x, y})] = TILE_ZERO + numBombs
			end
		end
	end

	print("After population")
	print(inspect(tileGrid))
end

local function populateStorageBuffer()
	local storageIndex = 1
	
	for y = 1, gridHeight do
		for x = 1, gridWidth do
			local tileTranslation = multiplyPairwise({x - 1, y - 1}, TILE_SIZE_PX)
			local tileScale = TILE_SIZE_PX

			-- TODO: use render grid in future, not tile grid
			local tileType = renderGrid[index2Dto1D({x, y})]
			-- Tile types are 0-indexed, convert to 1-indexed
			local tileData = atlasConverted[tileType + 1]

			local textureTranslation = tileData.translation
			local textureScale = tileData.scale

			storageBufferData[storageIndex + 0] = tileTranslation[X_COORD]
			storageBufferData[storageIndex + 1] = tileTranslation[Y_COORD]

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

	print("On submit called")
	entity0 = vGetEntityByID(0)
	entity1 = vGetEntityByID(1)
	entity2 = vGetEntityByID(2)
	entity3 = vGetEntityByID(3)
	entity4 = vGetEntityByID(4)
	entity5 = vGetEntityByID(5)

	vSetBufferData({
		0.0,	0.0,  0.0, 0.0,
		1.0,	0.0,  1.0, 0.0,
		1.0,	1.0,  1.0, 1.0,
		0.0,	1.0,  0.0, 1.0
	}, vFLOAT, entity2)

	vSetBufferData({
		0, 1, 2, 2, 3, 0
	}, vUINT16, entity3)

	for i = 1, gridSize * 8 do
		storageBufferData[i] = 0.0
	end

	vSetBufferData(storageBufferData, vFLOAT, entity4)
end

local function revealZeros(index2D)
	-- TODO: clean up some of this logic
	local index1D = index2Dto1D(index2D)

	renderGrid[index1D] = tileGrid[index1D]

	if tileGrid[index1D] ~= TILE_ZERO then
		return
	end

	for dy = -1, 1 do
		local ny = index2D[Y_COORD] + dy

		if ny <= 0 or ny > gridHeight then
			goto continueY
		end

		for dx = -1, 1 do
			local nx = index2D[X_COORD] + dx

			if nx <= 0 or nx > gridWidth then
				goto continueX
			end

			if dx == 0 and dy == 0 then
				goto continueX
			end

			local neighbourIndex = index2Dto1D({ nx, ny })

			if renderGrid[neighbourIndex] == TILE_UNREVEALED then
				print("Expanding the search")
				revealZeros({ nx, ny })
			end

			::continueX::
		end

		::continueY::
	end
end

local function getHoveredTile2D()
	local cursor = vCursorPosition()
	local tilePosition = multiplyPairwise(cursor, invert(TILE_SIZE_PX))
	local selectedTileX = math.ceil(tilePosition[X_COORD])
	local selectedTileY = math.ceil(tilePosition[Y_COORD])

	if selectedTileX <= 0 or selectedTileX > gridWidth then
		return nil
	end

	if selectedTileY <= 0 or selectedTileY > gridHeight then
		return nil
	end

	return { selectedTileX, selectedTileY }
end

local function handleInput()
	local hovered = getHoveredTile2D()

	if hovered == nil then
		return
	end

	local tileIndex1D = index2Dto1D(hovered)

	if vIsLeftClick() then
		if not hasRevealedBoard then
			hasRevealedBoard = true
			createGrid()
			populateBombs(hovered)
		end

		local revealedTile = tileGrid[tileIndex1D]
		local renderedTile = renderGrid[tileIndex1D]

		if revealedTile == TILE_BOMB then
			-- TODO: have to end game, for now just reveal entire grid
			for i = 1, gridSize do
				-- TODO: should hightlight the bomb we clicked?
				renderGrid[i] = tileGrid[i]
			end

			hasRevealedBoard = false

			return
		end

		if renderedTile == TILE_FLAG then
			renderGrid[tileIndex1D] = TILE_UNREVEALED

			return
		end

		renderGrid[tileIndex1D] = revealedTile

		-- TODO: additional floodfill logic when we find a 0
		if revealedTile == TILE_ZERO then
			revealZeros(hovered)
		end
	end

	if vIsRightClick() then
		local renderedTile = renderGrid[tileIndex1D]

		if renderedTile == TILE_UNREVEALED then
			renderGrid[tileIndex1D] = TILE_FLAG
		elseif renderedTile == TILE_FLAG then
			renderGrid[tileIndex1D] = TILE_UNREVEALED
		end
	end
end

local function vUpdate()
	-- TODO: shouldn't do every frame, jsut when theres a change
	handleInput()
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