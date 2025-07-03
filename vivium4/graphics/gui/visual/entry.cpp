#include "entry.h"

namespace Vivium {
	IntegerTextEntry submitEntry(EntrySpecification<IntegerTextEntry> const& specification, GUIContext& context, ResourceManager& resourceManager)
	{
		IntegerTextEntry entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.placeholder = specification.placeholder;
		entry.currentValue = "";
		entry.inputArea = submitButton(
			resourceManager,
			context,
			ButtonSpecification(entry.base, Color(0.15f, 0.15f, 0.15f), Color(0.0f, 0.0f, 0.0f))
		);

		return entry;
	}
	FloatTextEntry submitEntry(EntrySpecification<FloatTextEntry> const& specification, GUIContext& context, ResourceManager& resourceManager)
	{
		FloatTextEntry entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.placeholder = specification.placeholder;
		entry.currentValue = "";
		entry.inputArea = submitButton(
			resourceManager,
			context,
			ButtonSpecification(entry.base, Color(0.15f, 0.15f, 0.15f), Color(0.0f, 0.0f, 0.0f))
		);

		return entry;
	}

	StringTextEntry submitEntry(EntrySpecification<StringTextEntry> const& specification, GUIContext& context, ResourceManager& resourceManager)
	{
		StringTextEntry entry;

		entry.base = createGUIElement(context, GUIElementType::ENTRY);
		entry.placeholder = specification.placeholder;
		entry.currentValue = "";
		entry.inputArea = submitButton(
			resourceManager,
			context,
			ButtonSpecification(entry.base, Color(0.15f, 0.15f, 0.15f), Color(0.0f, 0.0f, 0.0f))
		);

		return entry;
	}


	void setupEntry(IntegerTextEntry& entry, ResourceManager& resourceManager, Engine& engine, CommandContext& context, GUIContext& guiContext)
	{
		setupButton(entry.inputArea, resourceManager);
	}

	void setupEntry(FloatTextEntry& entry, ResourceManager& resourceManager, Engine& engine, CommandContext& context, GUIContext& guiContext)
	{
		setupButton(entry.inputArea, resourceManager);
	}

	void setupEntry(StringTextEntry& entry, ResourceManager& resourceManager, Engine& engine, CommandContext& context, GUIContext& guiContext)
	{
		setupButton(entry.inputArea, resourceManager);
	}

	void updateEntry(IntegerTextEntry& entry, GUIContext& guiContext, Engine& engine, CommandContext& context)
	{
		// TODO: deal with backspace and all the text handling stuff
		//	should only enter character if cursor hovering/clicked/selected
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

				VIVIUM_LOG(LogSeverity::DEBUG, "Printable but not digit: {}", codepoint);
			}
			else {
				VIVIUM_LOG(LogSeverity::WARN, "Received unprintable character codepoint {}", codepoint);
			}
		}

		// TODO: only update text if change was made
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

	template <FiniteObjectType T>
	ObjectEntry<T> submitObjectEntry(GUIContext& context, ResourceManager& resourceManager, T const& defaultValue, std::vector<T> const& options);
	template <FiniteObjectType T>
	void setupEntry(ObjectEntry<T>& entry, ResourceManager& manager);
	template <FiniteObjectType T>
	void updateEntry(ObjectEntry<T>& entry, GUIContext& context);
}
