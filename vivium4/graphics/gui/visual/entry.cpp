#include "entry.h"

namespace Vivium {
	IntegerTextEntry submitIntegerTextEntry(std::string placeholder, GUIContext& context, ResourceManager& resourceManager)
	{
		IntegerTextEntry entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.placeholder = placeholder;
		entry.currentValue = "";
		entry.inputArea = submitButton(
			resourceManager,
			context,
			ButtonSpecification(entry.base, Color(0.15f, 0.15f, 0.15f), Color(0.0f, 0.0f, 0.0f))
		);
	
		return entry;
	}

	FloatTextEntry submitFloatTextEntry(std::string placeholder, GUIContext& context, ResourceManager& resourceManager)
	{
		FloatTextEntry entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.placeholder = placeholder;
		entry.currentValue = "";
		entry.inputArea = submitButton(
			resourceManager,
			context,
			ButtonSpecification(entry.base, Color(0.15f, 0.15f, 0.15f), Color(0.0f, 0.0f, 0.0f))
		);

		return entry;
	}

	StringTextEntry submitStringTextEntry(std::string placeholder, GUIContext& context, ResourceManager& resourceManager)
	{
		StringTextEntry entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.placeholder = placeholder;
		entry.currentValue = "";
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

	void updateEntry(IntegerTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
	{
		// TODO
		Input::CharacterData data = Input::getCharacters();

		for (uint64_t i = 0; i < data.size; i++) {
			uint32_t codepoint = data.codepoints[i];

			char character = static_cast<char>(codepoint);
			
			if (isprint(character)) {
				// TODO: right now we jsut pretend every codepoint is a valid ASCII character
				// We want to only consider numbers
				// TODO: deal with negatives as well

				if (isdigit(character)) {
					entry.currentValue += character;
				}
			}
			else {
				VIVIUM_LOG(LogSeverity::WARN, "Received unprintable character codepoint {}", codepoint);
			}
		}

		setButtonText(entry.inputArea, engine, context, guiContext, entry.currentValue);
	}

	void updateEntry(FloatTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
	{
		// TODO
	}

	void updateEntry(StringTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
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

	void dropEntry(IntegerTextEntry& entry, Engine& engine, GUIContext& guiContext)
	{
		dropButton(entry.inputArea, engine, guiContext);
	}

	void dropEntry(FloatTextEntry& entry, Engine& engine, GUIContext& guiContext)
	{
		dropButton(entry.inputArea, engine, guiContext);
	}

	void dropEntry(StringTextEntry& entry, Engine& engine, GUIContext& guiContext)
	{
		dropButton(entry.inputArea, engine, guiContext);
	}
}
