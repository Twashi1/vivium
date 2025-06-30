#pragma once

#include "context.h"
#include "panel.h"
#include "button.h"
#include "container.h"
#include "../../../input.h"

// assume an object entry is entered by a list of values (otherwise its a list/float/string)
//	we need that list of values to display/select from somehow

// Text input
//	arbitrary restriction function on characters that can be entered, and current data entered
// Enum input (select from different values with drop down)
// List input
//	composed of some other input, storing multiple alterable copies of that other input
//	or storing multiple inputs (that are re-arrangable)

/*
IntTextInput t = createIntTextInput(placeholderNumber)

updateTextInput(t)
getTextInput(t) -> int

ObjectInput<ShaderData> o = createObjectInput<ShaderData>(placeHolderValue);

updateObjectInput<ShaderData>(t)
getObjectInput<ShaderData>(t) -> ShaderData

ListInput<ObjectInput<ShaderData>> list = create....

updateListInput...
	- need general update function


	- need general get returning predictable type

submitInput (for rendering)
renderInput
*/

namespace Vivium {
	template <typename T>
	concept BaseEntry = requires (T & a, T const& b, char inputChar, GUIContext & context, std::span<T*> const span) {
		typename T::ValueType;
		{ getValue(b) } -> std::same_as<typename T::ValueType>;
		// TODO: update this for text entries additional parameters
		{ updateEntry(a, context) } -> std::same_as<void>;
		{ submitEntries(span, context) } -> std::same_as<void>;
	};

	template <typename T>
	struct TextEntry {
		using ValueType = T;

		GUIElementReference base;

		std::string placeholder;
		std::string currentValue;

		// TODO: bastardised button to mix text and panel
		Button inputArea;
	};

	typedef TextEntry<int> IntegerTextEntry;
	typedef TextEntry<float> FloatTextEntry;
	typedef TextEntry<std::string> StringTextEntry;

	IntegerTextEntry submitIntegerTextEntry(std::string placeholder, GUIContext& context, ResourceManager& resourceManager);
	FloatTextEntry submitFloatTextEntry(std::string placeholder, GUIContext& context, ResourceManager& resourceManager);
	StringTextEntry submitStringTextEntry(std::string placeholder, GUIContext& context, ResourceManager& resourceManager);

	void setupTextEntry(IntegerTextEntry& entry, ResourceManager& resourceManager);
	void setupTextEntry(FloatTextEntry& entry, ResourceManager& resourceManager);
	void setupTextEntry(StringTextEntry& entry, ResourceManager& resourceManager);

	void updateEntry(IntegerTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
	void updateEntry(FloatTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);
	void updateEntry(StringTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context);

	void submitEntries(std::span<IntegerTextEntry*> const entries, GUIContext& context);
	void submitEntries(std::span<FloatTextEntry*> const entries, GUIContext& context);
	void submitEntries(std::span<StringTextEntry*> const entries, GUIContext& context);

	void dropEntry(IntegerTextEntry& entry, Engine& engine, GUIContext& guiContext);
	void dropEntry(FloatTextEntry& entry, Engine& engine, GUIContext& guiContext);
	void dropEntry(StringTextEntry& entry, Engine& engine, GUIContext& guiContext);

	template <typename T>
	concept FiniteObjectType = requires (T a, T b) {
		std::is_copy_assignable_v<T>;
		{ a == b } -> std::same_as<bool>;
		{ getString(a) } -> std::convertible_to<char const*>;
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

	template <FiniteObjectType T>
	ObjectEntry<T> submitObjectEntry(GUIContext& context, ResourceManager& resourceManager, T const& defaultValue, std::vector<T> const& options)
	{
		ObjectEntry<T> entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.defaultValue = defaultValue;
		entry.currentlySelected = entry.defaultValue;
		entry.dropDownOpen = false;
		
		entry.objectView = submitButton(resourceManager, context, ButtonSpecification(
			entry.base,
			Color(0.25f, 0.25f, 0.25f),
			Color(0.0f, 0.0f, 0.0f)
		));

		entry.dropDownContainer = createContainer(context, ContainerSpecification(entry.base, ContainerOrdering::VERTICAL));
		entry.options = options;
		entry.dropDownOptions.reserve(options.size());

		Color changingColor = Color(0.5f, 0.5f, 0.5f);

		for (T const& value_type : options) {
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
		props.dimensions = F32x2(1.0f, 0.5f);
	}

	template <FiniteObjectType T>
	void updateEntry(ObjectEntry<T>& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
	{
		// TODO: do we need this line? and the one below
		setButtonText(entry.objectView, engine, context, guiContext, getString(entry.currentlySelected));

		for (uint64_t i = 0; i < entry.options.size(); i++) {
			setButtonText(entry.dropDownOptions[i], engine, context, guiContext, getString(entry.options[i]));
		}

		// If we click any button on the drop down menu we change the text of the objectView
		bool clicked = Input::get(Input::BTN_1).state == Input::RELEASE;
		bool hoveringObjectView = pointInElement(Input::getCursor(), properties(entry.objectView.base, guiContext));
		bool hoveringDropDown = pointInExtent(Input::getCursor(), properties(entry.dropDownContainer.base, guiContext));

		// If the drop down is open but our cursor isn't on it, or the object view, close it
		if (entry.dropDownOpen && !(hoveringObjectView || hoveringDropDown)) {
			entry.dropDownOpen = false;

			VIVIUM_LOG(LogSeverity::DEBUG, "Closing drop down");

			return;
		}

		// If drop down is closed and we click object view
		if (!entry.dropDownOpen && clicked && hoveringObjectView) {
			entry.dropDownOpen = true;

			VIVIUM_LOG(LogSeverity::DEBUG, "Opening drop down");

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
					char const* optionRepresentation = getString(entry.options[i]);

					// TODO: this seems to be the only set text thats working?
					setButtonText(entry.objectView, engine, context, guiContext, optionRepresentation);
					entry.currentlySelected = entry.options[i];

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

	template <BaseEntry ValueEntry>
	struct ListEntry {
		using ValueType = std::vector<ValueEntry>;
	};

	template <BaseEntry ValueEntry>
	ListEntry<ValueEntry>::ValueType getValue(ListEntry<ValueEntry> const& entry);
}