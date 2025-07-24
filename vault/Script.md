## Classes

`vEntity`
`vBuffer`
## Vivium functions

`vCreateEntity() -> Entity`
> Create an entity
> `SUBMIT`

`vGetEntityByName(string name) -> vEntity`
> Get an entity by name
> `SUBMIT`

`vGetComponent(vBUFFER, vEntity entity) -> vBuffer`
> Get a component on an entity
> `ANY`

`vSetComponent(vBuffer buffer, vEntity entity) -> nil`
> Set a component, or add a component to an entity if it doesn't already have it
> `SUBMIT, SETUP, UPDATE`

`vUploadBuffer(vBuffer buffer) -> nil`
> Update/transfer the given buffer to GPU memory
> `SETUP, UPDATE`

`vDraw(vPipeline) -> nil`
> Set a pipeline to be drawn next frame
> `DRAW`
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