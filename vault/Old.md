How do we organise the game engine?
- ideally we want to create something unique
- and with minimal overhead
- ideal would be to directly compile to `C++` the construction of objects
	- but seems complicated in practice?
- better would be to compile to some intermediary
	- optimise/interpret this intermediary representation
	- can then compile intermediary in future, but for now direct interpretation seems much easier

How does the user interact with the game engine?
- user creates entities
- user is responsible themselves for creation of the pipeline
	- they construct and attach shaders
	- any relevant shader resources are attached to a descriptor set, which is then attached to the pipeline with a matching descriptor layout
- representation of draw commands?
	- need some submission of a buffer to the shader layout/for rasterization
	- any given pipeline also has a buffer layout that the buffer must match
	- but we just also attach the buffer to the pipeline
	- when we draw, we have to select the relevant buffer/descriptor to bind

To create
- ability to add different entities
- modify individual instances with given properties
	- enter integer, string, list data
- drag and drop to add to hierarchy (like add element to inspector window)
- expand/hide child elements
- given some tree of children, interpret it to create elements
	- parse the tree somehow?

How does rendering properties, entering properties, and creating the properties menu/storing properties for some type of object work
- for the game engine we'd organise `Vulkan` objects as their own individual components
- each component has a set of values we'd need to modify
	- we hardcode the UI for these components?
	- we have list/string entries
		- we need this list UI to take arbitrary data
		- we need some drop-down menu functionality for selecting the `ShaderDataType`
		- list UI has a generalised display/entry mechanism (plus remove/organise buttons ideally)
	- the UI itself stores the values for these components?

We don't actually have a way to change a container's size based on its children

To build a real UI quickly an efficiently
- need some language with which to describe the UI quickly
- the language must incorporate almost any expected feature very easily
- intuitively build UI hierarchy

Say we were to re-design the UI around a different philosophy
- vast majority of UIs have a set font size and size of elements, with areas changing shapes to fit
- we can support the sizing thing, but we can't modify areas to change shape to fit
- we also can't make a container base its size off its children
- "fit sizing" -> "grow sizing" -> "positions" -> "draw"
- break up GUI update function a little to make it simpler
- clipping of text? more text functions for organising and controlling text

We use the ECS to store most relevant data
- each entity has components for what data it has on it
- pipeline objects require links
	- buffer specification
	- shaders
	- etc.
- we create these links by children or by drag and drop
	- just look at each child of the tree container, get the relevant entity for that panel
	- get the components for that entity
	- see which ones fit our needs for that pipeline's links
- an entry can link to a different entity?