## Classes

`vEntity`

`vComponentType`
`vDataType`
## Vivium functions

### Implemented

`vCreateEntity() -> vEntity`
> Create an entity
> `SUBMIT`

`vGetEntityByID(uint32) -> vEntity`
> Get entity by ID (should be removed later)
> `ANY`

### Partial functionality

`vSetBufferData(table arrayData, vDataType elementType, vEntity entity) -> nil`
> Modify the data passed as arrayData
> We assume arrayData starts at index 1 at can be incrementally indexed
> `SUBMIT, SETUP, UPDATE`, privileged in `SUBMIT`

`vCursorPosition() -> { number, number }`
> Retrieves the position of the cursor in (TODO: coordinate system?)
> `ANY`

`vIsLeftClick() -> bool`
> Is left mouse button clicked
> `ANY`

`vIsRightClick() -> bool`
> Is right mouse button clicked
> `ANY`
### TODO

`vGetEntityByName(string name) -> vEntity`
> Get an entity by name
> `ANY`

`vGetComponent(vComponentType, vEntity entity) -> ?`
> Get a component on an entity
> TODO: not necessary with how API is shaping up to be other than for debug/print purposes?
> `ANY`

`vSetBufferUsage(vBufferUsage, vEntity entity) -> nil`
> Set the usage of a buffer
> `SUBMIT`

`vAddComponent(vComponentType, vEntity entity) -> nil`
> Add a component to an entity if it doesn't already have it
> `SUBMIT, SETUP, UPDATE`

`vUploadBuffer(vEntity entity) -> nil`
> Update/transfer the given buffer to GPU memory
> `SETUP, UPDATE`

`vDraw(vPipeline) -> nil`
> Set a pipeline to be drawn next frame
> `DRAW`

`vDrawIndex(vEntity entity, int instanceCount) -> nil`
>Draw indices with given number of instances
>`DRAW`
## Special expected functions

`vSubmit`
> For all initialisation; creation of new components and entities

`vSetup`
> After allocation, perform any uploads of buffer/texture data

`vUpdate`
> Uploading new buffer data and performing per-frame updates

`vDraw`
> Scheduling any pipelines to be drawn

`vDrop`
> Any functionality required just before deleting any components