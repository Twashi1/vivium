#pragma once

#include "context.h"
#include "panel.h"
#include "button.h"
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
		{ getObjectSet<T>() } -> std::same_as<std::span<T const> const>;
		std::is_copy_assignable_v<T>;
		{ a == b } -> std::same_as<bool>;
	};

	template <FiniteObjectType T>
	struct ObjectEntry {
		using ValueType = T;
	};

	template <FiniteObjectType ObjectType>
	ObjectEntry<ObjectType>::ValueType getValue(ObjectEntry<ObjectType> const& entry);

	template <BaseEntry ValueEntry>
	struct ListEntry {
		using ValueType = std::vector<ValueEntry>;
	};

	template <BaseEntry ValueEntry>
	ListEntry<ValueEntry>::ValueType getValue(ListEntry<ValueEntry> const& entry);
}