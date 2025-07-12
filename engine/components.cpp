#include "components.h"

UniformBinding getValue(UniformBindingEntry& entry)
{
	UniformBinding binding;

	binding.slot = getValue(entry.slotEntry);
	binding.stage = getValue(entry.stageEntry);
	binding.type = getValue(entry.typeEntry);
	
	return binding;
}

UniformBindingEntry submitEntry(EntrySpecification<UniformBindingEntry> const& spec, GUIContext& guiContext, ResourceManager& resourceManager)
{
	UniformBindingEntry entry;

	entry.base = createGUIElement(guiContext, GUIElementType::ENTRY);
	entry.slotEntry = submitEntry(EntrySpecification<IntegerTextEntry>("Enter slot"), guiContext, resourceManager);

	entry.stageEntry = submitEntry(EntrySpecification<ObjectEntry<ShaderStage>>(
		ShaderStage::VERTEX,
		std::vector<ShaderStage>({ ShaderStage::VERTEX, ShaderStage::FRAGMENT })
	), guiContext, resourceManager);

	entry.typeEntry = submitEntry(EntrySpecification<ObjectEntry<UniformType>>(
		UniformType::STORAGE_BUFFER,
		std::vector<UniformType>({ UniformType::TEXTURE, UniformType::STORAGE_BUFFER, UniformType::UNIFORM_BUFFER })
	), guiContext, resourceManager);

	entry.background = createPanel(guiContext, PanelSpecification(
		entry.base,
		Color(0.1f, 0.1f, 0.1f),
		Color(0.0f, 0.0f, 0.0f),
		0.0f
	));

	entry.entryContainer = createContainer(guiContext, ContainerSpecification(
		entry.base,
		ContainerOrdering::VERTICAL,
		OffsetMethod::EXTENT
	));

	return entry;
}

void setupEntry(UniformBindingEntry& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
{
	// TODO: setup properties of everything

	setupEntry(entry.slotEntry, manager, engine, context, guiContext);
	setupEntry(entry.stageEntry, manager, engine, context, guiContext);
	setupEntry(entry.typeEntry, manager, engine, context, guiContext);

	addChild(entry.entryContainer.base, { &entry.slotEntry.base, 1 }, guiContext);
	addChild(entry.entryContainer.base, { &entry.stageEntry.base, 1 }, guiContext);
	addChild(entry.entryContainer.base, { &entry.typeEntry.base, 1 }, guiContext);

	properties(entry.background.base, guiContext).dimensions = F32x2(1.0f, 5.0f);
	properties(entry.background.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.background.base, guiContext).centerY = GUIAnchor::TOP;

	properties(entry.entryContainer.base, guiContext).dimensions = F32x2(0.9f);
	properties(entry.entryContainer.base, guiContext).position = F32x2(0.0f, -0.1f);
	properties(entry.entryContainer.base, guiContext).anchorY = GUIAnchor::TOP;
	properties(entry.entryContainer.base, guiContext).centerY = GUIAnchor::TOP;
}

void updateEntry(UniformBindingEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
{
	updateEntry(entry.slotEntry, guiContext, engine, context);
	updateEntry(entry.stageEntry, guiContext, engine, context);
	updateEntry(entry.typeEntry, guiContext, engine, context);
}

void submitEntries(std::span<UniformBindingEntry*> const entries, GUIContext& guiContext)
{
	for (UniformBindingEntry* entry : entries) {
		IntegerTextEntry* slots[] = { &entry->slotEntry };
		ObjectEntry<UniformType>* types[] = { &entry->typeEntry };
		ObjectEntry<ShaderStage>* stages[] = { &entry->stageEntry };
		Panel* panels[] = { &entry->background };

		submitPanels(panels, guiContext);
		submitEntries(slots, guiContext);
		submitEntries<UniformType>(types, guiContext);
		submitEntries<ShaderStage>(stages, guiContext);
	}
}

void dropEntry(UniformBindingEntry& entry, Engine& engine, GUIContext& guiContext)
{
	dropEntry(entry.slotEntry, engine, guiContext);
	dropEntry(entry.typeEntry, engine, guiContext);
	dropEntry(entry.stageEntry, engine, guiContext);
}