#pragma once

#include <regex>

#include "context.h"
#include "panel.h"
#include "button.h"
#include "container.h"
#include "../../../input.h"
#include "../../../ecs/defines.h"

namespace Vivium {
	// TODO: validation with concept
	template <typename EntryType>
	struct EntrySpecification;

	template <typename T>
	struct TextEntry {
		using ValueType = T;

		GUIElementReference base;

		std::string placeholder;
		std::string currentValue;
		std::string lastValidValue;

		// TODO: bastardised button to mix text and panel
		Button inputArea;

		bool entrySelected;
	};

	typedef TextEntry<int> IntegerTextEntry;
	typedef TextEntry<float> FloatTextEntry;
	typedef TextEntry<std::string> StringTextEntry;

	// TODO: get value methods for integer/float/string entries
	//	additionally must deal with the string having invalid characters

	std::string getString(Entity entity);

	template <typename T>
	concept FiniteObjectType = requires (T a, T b) {
		std::is_copy_assignable_v<T>;
		{ a == b } -> std::same_as<bool>;
		{ getString(a) } -> std::convertible_to<std::string>;
	};

	template <FiniteObjectType T>
	struct UploadEntry {
		using ValueType = T;

		GUIElementReference base;

		ValueType currentlySelected;
		std::string placeholder;
		ValueType** heldItemPointer;

		Panel valuePanel;
		Button valueButton;
		Panel clearPanel;

		bool hasValue;
	};

	template <FiniteObjectType T>
	struct ObjectEntry {
		using ValueType = T;

		GUIElementReference base;

		ValueType defaultValue;
		ValueType currentlySelected;

		Button objectView;
		Container dropDownContainer;
		bool dropDownOpen;

		std::vector<Button> dropDownOptions;
		std::vector<ValueType> options;

		// TODO: note we'd have to re-create this with new buttons if we change the option set

		// TODO: we need to create a drop-down menu
		//	so a vbox with a bunch of buttons with the representation of each element
		//	vbox might be big so we ideally want it to be scrollable?
		//	this would require clipping content
	};

	template <typename ValueEntry>
	struct ListEntry {
		using ValueType = std::vector<typename ValueEntry::ValueType>;

		GUIElementReference base;

		uint64_t numEntries;
		Button addEntry;
		Container entryContainer;
		std::vector<ValueEntry> entries;
		std::vector<Panel> deleteEntry;
		std::vector<Panel> entryUp;
		std::vector<Panel> entryDown;
		std::vector<Panel> entryWrapper;
	};

	// TODO: need more params probably
	//	- rendering settings?
	template <>
	struct EntrySpecification<IntegerTextEntry> {
		std::string placeholder;
	};

	template <>
	struct EntrySpecification<FloatTextEntry> {
		std::string placeholder;
	};

	template <>
	struct EntrySpecification<StringTextEntry> {
		std::string placeholder;
	};

	template <FiniteObjectType T>
	struct EntrySpecification<ObjectEntry<T>> {
		T defaultValue;
		std::vector<T> options;
	};

	template <FiniteObjectType T>
	struct EntrySpecification<UploadEntry<T>> {
		std::string placeholder;
		T** heldItemPointer;
	};

	template <typename ValueEntry>
	struct EntrySpecification<ListEntry<ValueEntry>> {
		uint64_t maxEntries;
		EntrySpecification<ValueEntry>* valueSpecification;
	};

	IntegerTextEntry submitEntry(EntrySpecification<IntegerTextEntry> const& specification, GUIContext& context, ResourceManager& resourceManager);
	FloatTextEntry submitEntry(EntrySpecification<FloatTextEntry> const& specification, GUIContext& context, ResourceManager& resourceManager);
	StringTextEntry submitEntry(EntrySpecification<StringTextEntry> const& specification, GUIContext& context, ResourceManager& resourceManager);

	void setupEntry(IntegerTextEntry& entry, ResourceManager& resourceManager, Engine& engine, CommandContext& context, GUIContext& guiContext);
	void setupEntry(FloatTextEntry& entry, ResourceManager& resourceManager, Engine& engine, CommandContext& context, GUIContext& guiContext);
	void setupEntry(StringTextEntry& entry, ResourceManager& resourceManager, Engine& engine, CommandContext& context, GUIContext& guiContext);

	void updateEntry(IntegerTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
	void updateEntry(FloatTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
	void updateEntry(StringTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);

	void submitEntries(std::span<IntegerTextEntry*> const entries, GUIContext& context);
	void submitEntries(std::span<FloatTextEntry*> const entries, GUIContext& context);
	void submitEntries(std::span<StringTextEntry*> const entries, GUIContext& context);

	void dropEntry(IntegerTextEntry& entry, Engine& engine, GUIContext& guiContext);
	void dropEntry(FloatTextEntry& entry, Engine& engine, GUIContext& guiContext);
	void dropEntry(StringTextEntry& entry, Engine& engine, GUIContext& guiContext);

	int getValue(IntegerTextEntry const& entry);
	float getValue(FloatTextEntry const& entry);
	std::string getValue(StringTextEntry const& entry);

	template <FiniteObjectType T>
	ObjectEntry<T> submitEntry(EntrySpecification<ObjectEntry<T>> const& specification, GUIContext& context, ResourceManager& resourceManager)
	{
		ObjectEntry<T> entry;

		entry.defaultValue = specification.defaultValue;
		entry.currentlySelected = entry.defaultValue;
		entry.dropDownOpen = false;
		
		entry.objectView = submitButton(resourceManager, context, ButtonSpecification(
			nullGUIParent(),
			Color(0.25f, 0.25f, 0.25f),
			Color(0.0f, 0.0f, 0.0f)
		));
		entry.base = entry.objectView.base;

		entry.dropDownContainer = createContainer(context, ContainerSpecification(entry.objectView.base, ContainerOrdering::VERTICAL, OffsetMethod::EXTENT));
		entry.options = specification.options;
		entry.dropDownOptions.reserve(entry.options.size());

		Color changingColor = Color(0.5f, 0.5f, 0.0f);

		for (T const& value_type : entry.options) {
			Button dropDownOption;

			dropDownOption = submitButton(resourceManager, context, ButtonSpecification(
				entry.dropDownContainer.base,
				changingColor,
				Color(0.0f, 0.0f, 0.0f)
			));

			changingColor.b += 0.1f;

			entry.dropDownOptions.push_back(dropDownOption);
		}

		return entry;
	}

	template <FiniteObjectType T>
	void setupEntry(ObjectEntry<T>& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
	{
		setupButton(entry.objectView, manager);

		for (uint64_t i = 0; i < entry.options.size(); i++) {
			setupButton(entry.dropDownOptions[i], manager);
			setButtonText(entry.dropDownOptions[i], engine, context, guiContext, getString(entry.options[i]));
		}

		setButtonText(entry.objectView, engine, context, guiContext, getString(entry.defaultValue));

		GUIProperties& props = properties(entry.dropDownContainer.base, guiContext);
		props.anchorY = GUIAnchor::BOTTOM;
		props.centerY = GUIAnchor::TOP;
		props.dimensions = F32x2(0.9f, 1.0f);
		props.position = F32x2(0.0f, -0.1f);
	}

	template <FiniteObjectType T>
	void updateEntry(ObjectEntry<T>& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
	{
		setButtonText(entry.objectView, engine, context, guiContext, getString(entry.currentlySelected));

		for (uint64_t i = 0; i < entry.options.size(); i++) {
			setButtonText(entry.dropDownOptions[i], engine, context, guiContext, getString(entry.options[i]));
		}

		// If we click any button on the drop down menu we change the text of the objectView
		bool clicked = Input::get(Input::BTN_1).state == Input::RELEASE;
		bool hoveringObjectView = pointInElement(Input::getCursor(), properties(entry.objectView.base, guiContext));
		bool hoveringDropDown = pointInExtent(Input::getCursor(), properties(entry.dropDownContainer.base, guiContext));

		setAsleep(entry.dropDownContainer.base, guiContext, !entry.dropDownOpen);

		// If the drop down is open but our cursor isn't on it, or the object view, close it
		if (entry.dropDownOpen && !(hoveringObjectView || hoveringDropDown)) {
			entry.dropDownOpen = false;

			return;
		}

		// If drop down is closed and we click object view
		if (!entry.dropDownOpen && clicked && hoveringObjectView) {
			entry.dropDownOpen = true;

			return;
		}

		// If drop down is open and we clicked on it
		if (entry.dropDownOpen && hoveringDropDown && clicked) {
			// Find which we clicked
			for (uint64_t i = 0; i < entry.options.size(); i++) {
				Button& button = entry.dropDownOptions[i];

				bool hoveringButton = pointInElement(Input::getCursor(), properties(button.base, guiContext));

				// This is the button we clicked
				if (hoveringButton) {
					// Get string for the option we clicked
					std::string optionRepresentation = getString(entry.options[i]);

					// TODO: this seems to be the only set text thats working?
					setButtonText(entry.objectView, engine, context, guiContext, optionRepresentation);
					entry.currentlySelected = entry.options[i];

					entry.dropDownOpen = false;

					return;
				}
			}
		}
	}

	template <FiniteObjectType T>
	void submitEntries(std::span<ObjectEntry<T>*> const entries, GUIContext& context)
	{
		for (ObjectEntry<T>* entry : entries) {
			std::vector<Button*> buttons;
			buttons.push_back(&entry->objectView);

			if (entry->dropDownOpen) {
				for (Button& button : entry->dropDownOptions) {
					buttons.push_back(&button);
				}
			}

			submitButtons(buttons, context);
		}
	}
	
	template <FiniteObjectType T>
	void dropEntry(ObjectEntry<T>& entry, Engine& engine, GUIContext& guiContext)
	{
		dropButton(entry.objectView, engine, guiContext);

		for (Button& button : entry.dropDownOptions) {
			dropButton(button, engine, guiContext);
		}
	}

	template <FiniteObjectType ObjectType>
	ObjectEntry<ObjectType>::ValueType getValue(ObjectEntry<ObjectType> const& entry)
	{
		return entry.currentlySelected;
	}

	template <FiniteObjectType T>
	UploadEntry<T> submitEntry(EntrySpecification<UploadEntry<T>> const& specification, GUIContext& context, ResourceManager& resourceManager)
	{
		UploadEntry<T> entry;

		entry.valuePanel = createPanel(context, PanelSpecification(
			nullGUIParent(),
			Color(0.25f, 0.25f, 0.25f),
			Color(0.1f, 0.1f, 0.1f),
			0.05f
		));
		entry.base = entry.valuePanel.base;
		entry.valueButton = submitButton(resourceManager, context, ButtonSpecification(
			entry.valuePanel.base,
			Color(0.25f, 0.25f, 0.25f),
			Color(0.0f, 0.0f, 0.0f)
		));

		entry.clearPanel = createPanel(context, PanelSpecification(
			entry.valuePanel.base,
			Color(0.85f, 0.25f, 0.25f),
			Color(0.1f, 0.1f, 0.1f),
			0.05f
		));
		entry.hasValue = false;
		entry.placeholder = specification.placeholder;
		entry.heldItemPointer = specification.heldItemPointer;

		VIVIUM_ASSERT(entry.heldItemPointer != nullptr, "No pointer to where held item is");

		return entry;
	}

	template <FiniteObjectType T>
	void setupEntry(UploadEntry<T>& entry, ResourceManager& manager, Engine& engine, CommandContext& context, GUIContext& guiContext)
	{
		setupButton(entry.valueButton, manager);
		setButtonText(entry.valueButton, engine, context, guiContext, entry.placeholder);

		// TODO: anchor to left?
		properties(entry.valueButton, guiContext).dimensions = F32x2(0.65f, 1.0f);
		
		properties(entry.clearPanel, guiContext).anchorX = GUIAnchor::RIGHT;
		properties(entry.clearPanel, guiContext).centerX = GUIAnchor::RIGHT;
		properties(entry.clearPanel, guiContext).dimensions = F32x2(0.15f, 0.8f);
		properties(entry.clearPanel, guiContext).position = F32x2(-0.05f, 0.0f);
	}

	template <FiniteObjectType T>
	void updateEntry(UploadEntry<T>& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
	{
		// TODO: bad, switch on string not function
		if (entry.hasValue) {
			setButtonText(entry.valueButton, engine, context, guiContext,
				std::format("Entity {}", getString(entry.currentlySelected)));
		}
		else {
			setButtonText(entry.valueButton, engine, context, guiContext, entry.placeholder);
		}

		bool clicked = Input::get(Input::BTN_1).state == Input::RELEASE;
		bool hoveringUpload = pointInElement(Input::getCursor(), properties(entry.valueButton.base, guiContext));
		bool hoveringClear = pointInElement(Input::getCursor(), properties(entry.clearPanel.base, guiContext));

		if (clicked) {
			if (hoveringClear) {
				// Clear out the currently selected value
				entry.hasValue = false;
			}

			// Need to update the selected value
			if (hoveringUpload) {
				// Check we're holding something
				if (*entry.heldItemPointer != nullptr) {
					entry.currentlySelected = **entry.heldItemPointer;
					entry.hasValue = true;
				}
				// TODO: We should release the held item... how?
				//	could just leave it to the application being responsible for releasing it
			}
		}
	}

	template <FiniteObjectType T>
	void submitEntries(std::span<UploadEntry<T>*> const entries, GUIContext& context)
	{
		for (UploadEntry<T>* entry : entries) {
			Button* buttons[] = { &entry->valueButton };
			submitButtons(buttons, context);

			Panel* panels[] = { &entry->valuePanel, &entry->clearPanel };
			submitPanels(panels, context);
		}
	}

	template <FiniteObjectType T>
	void dropEntry(UploadEntry<T>& entry, Engine& engine, GUIContext& guiContext)
	{
		dropButton(entry.valueButton, engine, guiContext);
	}

	template <FiniteObjectType ObjectType>
	UploadEntry<ObjectType>::ValueType getValue(UploadEntry<ObjectType> const& entry)
	{
		// TODO: bit bad, only actually release selected value if there is one (hasValue)
		VIVIUM_ASSERT(entry.hasValue, "Entry didn't have value but we tried to request it");
		return entry.currentlySelected;
	}

	template <typename ValueEntry>
	ListEntry<ValueEntry> submitEntry(EntrySpecification<ListEntry<ValueEntry>> const& specification, GUIContext& context, ResourceManager& resourceManager)
	{
		ListEntry<ValueEntry> entry;

		entry.entryContainer = createContainer(context, ContainerSpecification(
			nullGUIParent(),
			ContainerOrdering::VERTICAL,
			OffsetMethod::EXTENT
		));
		entry.base = entry.entryContainer.base;

		entry.addEntry = submitButton(resourceManager, context, ButtonSpecification(
			entry.entryContainer.base,
			Color(0.25f, 0.65f, 0.25f),
			Color(0.0f, 0.0f, 0.0f)
		));
		entry.numEntries = 0;

		properties(entry.addEntry.base, context).dimensions = F32x2(1.0f, 0.15f);
		properties(entry.addEntry.base, context).anchorY = GUIAnchor::TOP;
		properties(entry.addEntry.base, context).centerY = GUIAnchor::TOP;

		entry.entries.resize(specification.maxEntries);
		entry.deleteEntry.resize(specification.maxEntries);
		entry.entryUp.resize(specification.maxEntries);
		entry.entryDown.resize(specification.maxEntries);
		entry.entryWrapper.resize(specification.maxEntries);

		for (uint64_t i = 0; i < specification.maxEntries; i++) {
			entry.entries[i] = submitEntry(*specification.valueSpecification, context, resourceManager);
			entry.entryWrapper[i] = createPanel(context, PanelSpecification(
				entry.entryContainer.base,
				Color(0.25f, 0.25f, 0.25f),
				Color(0.0f, 0.0f, 0.0f),
				0.01f
			));
			properties(entry.entryWrapper[i].base, context).anchorY = GUIAnchor::TOP;
			properties(entry.entryWrapper[i].base, context).centerY = GUIAnchor::TOP;

			entry.deleteEntry[i] = createPanel(context, PanelSpecification(
				entry.entryWrapper[i].base,
				Color(0.6f, 0.1f, 0.1f),
				Color(0.0f, 0.0f, 0.0f),
				0.01f
			));
			entry.entryUp[i] = createPanel(context, PanelSpecification(
				entry.entryWrapper[i].base,
				Color(0.6f, 0.6f, 0.6f),
				Color(0.0f, 0.0f, 0.0f),
				0.01f
			));
			entry.entryDown[i] = createPanel(context, PanelSpecification(
				entry.entryWrapper[i].base,
				Color(0.6f, 0.6f, 0.6f),
				Color(0.0f, 0.0f, 0.0f),
				0.01f
			));

			properties(entry.entryWrapper[i].base, context).dimensions = F32x2(1.0f, 0.15f);

			properties(entry.deleteEntry[i].base, context).dimensions = F32x2(0.1f, 0.5f);
			properties(entry.entryUp[i].base, context).dimensions = F32x2(0.1f, 0.3f);
			properties(entry.entryDown[i].base, context).dimensions = F32x2(0.1f, 0.3f);
			properties(entry.entries[i].base, context).dimensions = F32x2(0.65f, 0.85f);

			properties(entry.deleteEntry[i].base, context).position = F32x2(0.1f, 0.0f);
			properties(entry.entryUp[i].base, context).position = F32x2(-0.1f, 0.2f);
			properties(entry.entryDown[i].base, context).position = F32x2(-0.1f, -0.2f);

			properties(entry.entryDown[i].base, context).anchorX = GUIAnchor::RIGHT;
			properties(entry.entryUp[i].base, context).anchorX = GUIAnchor::RIGHT;
			properties(entry.deleteEntry[i].base, context).anchorX = GUIAnchor::LEFT;

			properties(entry.entryDown[i].base, context).centerX = GUIAnchor::RIGHT;
			properties(entry.entryUp[i].base, context).centerX = GUIAnchor::RIGHT;
			properties(entry.deleteEntry[i].base, context).centerX = GUIAnchor::LEFT;

			addChild(entry.entryWrapper[i].base, { &entry.entries[i].base, 1 }, context);

			setAsleep(entry.entries[i].base, context, true);
			setAsleep(entry.entryWrapper[i].base, context, true);
			setAsleep(entry.deleteEntry[i].base, context, true);
			setAsleep(entry.entryUp[i].base, context, true);
			setAsleep(entry.entryDown[i].base, context, true);
		}

		return entry;
	}
	
	template <typename ValueEntry>
	void setupEntry(ListEntry<ValueEntry>& entry, ResourceManager& resourceManager, Engine& engine, CommandContext& context, GUIContext& guiContext)
	{
		setupButton(entry.addEntry, resourceManager);
		setButtonText(entry.addEntry, engine, context, guiContext, "Add entry");

		for (ValueEntry& child : entry.entries) {
			setupEntry(child, resourceManager, engine, context, guiContext);
		}
	}

	template <typename ValueEntry>
	void _removeIndexFromListEntry(ListEntry<ValueEntry>& entry, GUIContext& guiContext, uint64_t index)
	{
		// what if we just moved that entry to the back of the list?
		//	- should also be resetting its value to the default?
		// also have to update the relevant container
	
		// TODO: how does tree container do it?
		//	this container operates very differently
		setAsleep(entry.entries[index].base, guiContext, true);
		setAsleep(entry.entryWrapper[index].base, guiContext, true);
		setAsleep(entry.deleteEntry[index].base, guiContext, true);
		setAsleep(entry.entryUp[index].base, guiContext, true);
		setAsleep(entry.entryDown[index].base, guiContext, true);

		// Move element to end while preserving order
		std::rotate(entry.entries.begin() + index, entry.entries.begin() + index + 1, entry.entries.end());
		std::rotate(entry.entryWrapper.begin() + index, entry.entryWrapper.begin() + index + 1, entry.entryWrapper.end());
		std::rotate(entry.deleteEntry.begin() + index, entry.deleteEntry.begin() + index + 1, entry.deleteEntry.end());
		std::rotate(entry.entryUp.begin() + index, entry.entryUp.begin() + index + 1, entry.entryUp.end());
		std::rotate(entry.entryDown.begin() + index, entry.entryDown.begin() + index + 1, entry.entryDown.end());
		// TODO: terrible code
		// NOTE: add 1 is because add entry is also stored in the container
		rotateChild(entry.entryContainer.base, index + 1, index + 2, getChildren(entry.entryContainer.base, guiContext).size(), guiContext);

		entry.numEntries--;
	}

	template <typename ValueEntry>
	void _insertIndexToListEntry(ListEntry<ValueEntry>& entry, GUIContext& guiContext, uint64_t index)
	{
		if (entry.numEntries == entry.entries.size()) {
			VIVIUM_LOG(LogSeverity::WARN, "List entry at max size");

			return;
		}

		// We need to rotate the last element into place
		std::rotate(entry.entries.begin() + index, entry.entries.end(), entry.entries.end());
		std::rotate(entry.entryWrapper.begin() + index, entry.entryWrapper.end(), entry.entryWrapper.end());
		std::rotate(entry.deleteEntry.begin() + index, entry.deleteEntry.end(), entry.deleteEntry.end());
		std::rotate(entry.entryUp.begin() + index, entry.entryUp.end(), entry.entryUp.end());
		std::rotate(entry.entryDown.begin() + index, entry.entryDown.end(), entry.entryDown.end());
		rotateChild(entry.entryContainer.base, index + 1, getChildren(entry.entryContainer.base, guiContext).size(), getChildren(entry.entryContainer.base, guiContext).size(), guiContext);

		setAsleep(entry.entries[index].base, guiContext, false);
		setAsleep(entry.entryWrapper[index].base, guiContext, false);
		setAsleep(entry.deleteEntry[index].base, guiContext, false);
		setAsleep(entry.entryUp[index].base, guiContext, false);
		setAsleep(entry.entryDown[index].base, guiContext, false);

		entry.numEntries++;
	}

	template <typename ValueEntry>
	void _swapIndicesListEntry(ListEntry<ValueEntry>& entry, GUIContext& guiContext, uint64_t a, uint64_t b)
	{
		if (a < 0 || b < 0) return;
		if (a >= entry.numEntries || b >= entry.numEntries) return;

		std::swap(entry.entries[a], entry.entries[b]);
		std::swap(entry.entryWrapper[a], entry.entryWrapper[b]);
		std::swap(entry.deleteEntry[a], entry.deleteEntry[b]);
		std::swap(entry.entryUp[a], entry.entryUp[b]);
		std::swap(entry.entryDown[a], entry.entryDown[b]);
		swapChildren(entry.entryContainer.base, a + 1, b + 1, guiContext);
	}

	template <typename ValueEntry>
	void updateEntry(ListEntry<ValueEntry>& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
	{
		// TODO: gotta find a work-around for this
		setButtonText(entry.addEntry, engine, context, guiContext, "Add entry");
		
		bool clicked = (Input::get(Input::BTN_1).state == Input::RELEASE);
		F32x2 cursor = Input::getCursor();

		if (clicked) {
			if (pointInElement(cursor, properties(entry.addEntry.base, guiContext))) {
				_insertIndexToListEntry(entry, guiContext, entry.numEntries);
			}
			else {
				for (uint64_t i = 0; i < entry.numEntries; i++) {
					if (pointInElement(cursor, properties(entry.deleteEntry[i].base, guiContext))) {
						_removeIndexFromListEntry(entry, guiContext, i);

						break;
					}
					else if (pointInElement(cursor, properties(entry.entryUp[i].base, guiContext))) {
						_swapIndicesListEntry(entry, guiContext, i, i - 1);


						break;
					}
					else if (pointInElement(cursor, properties(entry.entryDown[i].base, guiContext))) {
						_swapIndicesListEntry(entry, guiContext, i, i + 1);

						break;
					}
				}
			}
		}

		for (uint64_t i = 0; i < entry.numEntries; i++) {
			updateEntry(entry.entries[i], guiContext, engine, context);
		}
	}

	template <typename ValueEntry>
	void submitEntries(std::span<ListEntry<ValueEntry>*> const entries, GUIContext& context)
	{
		for (ListEntry<ValueEntry>* entry : entries) {
			Button* button[] = { &entry->addEntry };
			submitButtons(button, context);

			std::vector<Panel*> panels;
			std::vector<ValueEntry*> childEntries;

			for (uint64_t i = 0; i < entry->numEntries; i++) {
				panels.push_back(&entry->entryWrapper[i]);
				panels.push_back(&entry->deleteEntry[i]);
				panels.push_back(&entry->entryUp[i]);
				panels.push_back(&entry->entryDown[i]);
				childEntries.push_back(&entry->entries[i]);
			}

			submitPanels(panels, context);

			// TODO: not necessary likely
			std::span<ValueEntry*> childEntrySpan = childEntries;

			submitEntries(childEntrySpan, context);
		}
	}
	
	template <typename ValueEntry>
	void dropEntry(ListEntry<ValueEntry>& entry, Engine& engine, GUIContext& guiContext)
	{
		dropButton(entry.addEntry, engine, guiContext);

		for (ValueEntry& child : entry.entries) {
			dropEntry(child, engine, guiContext);
		}
	}

	template <typename ValueEntry>
	ListEntry<ValueEntry>::ValueType getValue(ListEntry<ValueEntry> const& entry)
	{
		std::vector<typename ValueEntry::ValueType> results;

		for (uint64_t i = 0; i < entry.numEntries; i++) {
			results.push_back(getValue(entry.entries[i]));
		}

		return results;
	}
}