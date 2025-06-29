#include "entry.h"

namespace Vivium {
	IntegerTextEntry createIntegerTextEntry(std::string placeholder, GUIContext& context, ResourceManager& resourceManager)
	{
		IntegerTextEntry entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.placeholder = placeholder;
		entry.currentValue = placeholder;
		entry.inputArea = submitButton(
			resourceManager,
			context,
			ButtonSpecification(entry.base, Color(0.15f, 0.15f, 0.15f), Color(0.0f, 0.0f, 0.0f))
		);
	
		return entry;
	}

	FloatTextEntry createFloatTextEntry(std::string placeholder, GUIContext& context, ResourceManager& resourceManager)
	{
		FloatTextEntry entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.placeholder = placeholder;
		entry.currentValue = placeholder;
		entry.inputArea = submitButton(
			resourceManager,
			context,
			ButtonSpecification(entry.base, Color(0.15f, 0.15f, 0.15f), Color(0.0f, 0.0f, 0.0f))
		);

		return entry;
	}

	StringTextEntry createStringTextEntry(std::string placeholder, GUIContext& context, ResourceManager& resourceManager)
	{
		StringTextEntry entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.placeholder = placeholder;
		entry.currentValue = placeholder;
		entry.inputArea = submitButton(
			resourceManager,
			context,
			ButtonSpecification(entry.base, Color(0.15f, 0.15f, 0.15f), Color(0.0f, 0.0f, 0.0f))
		);

		return entry;
	}

	void setupTextEntry(IntegerTextEntry& entry, ResourceManager& resourceManager)
	{
		setupButton(entry.inputArea, resourceManager);
	}

	void setupTextEntry(FloatTextEntry& entry, ResourceManager& resourceManager)
	{
		setupButton(entry.inputArea, resourceManager);
	}

	void setupTextEntry(StringTextEntry& entry, ResourceManager& resourceManager)
	{
		setupButton(entry.inputArea, resourceManager);
	}

	void updateEntry(IntegerTextEntry& entry, GUIContext& context)
	{
		// TODO
		Input::CharacterData data = Input::getCharacters();
	}

	void updateEntry(FloatTextEntry& entry, GUIContext& context)
	{
		// TODO
	}

	void updateEntry(StringTextEntry& entry, GUIContext& context)
	{
		// TODO
	}

	void submitEntries(std::span<IntegerTextEntry*> const entries, GUIContext& context)
	{
		for (IntegerTextEntry* entry : entries) {
			Button* entryList[] = {&entry->inputArea};
			submitButtons(entryList, context);
		}
	}

	void submitEntries(std::span<FloatTextEntry*> const entries, GUIContext& context)
	{
		for (FloatTextEntry* entry : entries) {
			Button* entryList[] = { &entry->inputArea };
			submitButtons(entryList, context);
		}
	}

	void submitEntries(std::span<StringTextEntry*> const entries, GUIContext& context)
	{
		for (StringTextEntry* entry : entries) {
			Button* entryList[] = { &entry->inputArea };
			submitButtons(entryList, context);
		}
	}
}
