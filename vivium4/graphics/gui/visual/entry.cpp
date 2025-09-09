#include "entry.h"

namespace Vivium {
std::string getString(Entity entity) {
  return std::to_string(getIdentifier(entity));
};

IntegerTextEntry submitEntry(
    EntrySpecification<IntegerTextEntry> const& specification,
    GUIContext& context, ResourceManager& resourceManager) {
  IntegerTextEntry entry;

  entry.placeholder = specification.placeholder;
  entry.currentValue = "";
  entry.lastValidValue = "0";
  entry.entrySelected = false;
  entry.inputArea = submitButton(
      resourceManager, context,
      ButtonSpecification(nullGUIParent(), Color(0.15f, 0.15f, 0.15f),
                          Color(0.0f, 0.0f, 0.0f)));
  entry.base = entry.inputArea.base;

  return entry;
}
FloatTextEntry submitEntry(
    EntrySpecification<FloatTextEntry> const& specification,
    GUIContext& context, ResourceManager& resourceManager) {
  FloatTextEntry entry;

  entry.placeholder = specification.placeholder;
  entry.currentValue = "";
  entry.lastValidValue = "0.0";
  entry.entrySelected = false;
  entry.inputArea = submitButton(
      resourceManager, context,
      ButtonSpecification(nullGUIParent(), Color(0.15f, 0.15f, 0.15f),
                          Color(0.0f, 0.0f, 0.0f)));
  entry.base = entry.inputArea.base;

  return entry;
}

StringTextEntry submitEntry(
    EntrySpecification<StringTextEntry> const& specification,
    GUIContext& context, ResourceManager& resourceManager) {
  StringTextEntry entry;

  entry.base = createGUIElement(context, GUIElementType::ENTRY);
  entry.placeholder = specification.placeholder;
  entry.currentValue = "";
  entry.lastValidValue = "";
  entry.entrySelected = false;
  entry.inputArea = submitButton(
      resourceManager, context,
      ButtonSpecification(nullGUIParent(), Color(0.15f, 0.15f, 0.15f),
                          Color(0.0f, 0.0f, 0.0f)));
  entry.base = entry.inputArea.base;

  return entry;
}

void setupEntry(IntegerTextEntry& entry, ResourceManager& resourceManager,
                Engine& engine, CommandContext& context,
                GUIContext& guiContext) {
  setupButton(entry.inputArea, resourceManager);
}

void setupEntry(FloatTextEntry& entry, ResourceManager& resourceManager,
                Engine& engine, CommandContext& context,
                GUIContext& guiContext) {
  setupButton(entry.inputArea, resourceManager);
}

void setupEntry(StringTextEntry& entry, ResourceManager& resourceManager,
                Engine& engine, CommandContext& context,
                GUIContext& guiContext) {
  setupButton(entry.inputArea, resourceManager);
}

void updateEntry(IntegerTextEntry& entry, GUIContext& guiContext,
                 Engine& engine, CommandContext& context) {
  bool clicked = (Input::get(Input::BTN_1).state == Input::RELEASE);
  bool hovered =
      pointInElement(Input::getCursor(), properties(entry.base, guiContext));
  bool entered = (Input::get(Input::KEY_ENTER).state == Input::RELEASE);

  // TODO: deal with backspace and all the text handling stuff
  //	should only enter character if cursor hovering/clicked/selected
  if (clicked && hovered) {
    entry.entrySelected = true;
    entry.currentValue = "";
  }

  if ((clicked && !hovered) || entered) {
    entry.entrySelected = false;

    // TODO: do work to submit vlaue
    // Integer must be composed of just digits
    std::regex intPattern("-?\\d+");

    if (std::regex_match(entry.currentValue, intPattern)) {
      entry.lastValidValue = entry.currentValue;
    } else {
      VIVIUM_LOG(LogSeverity::WARN, "Invalid input entered: {}",
                 entry.currentValue);

      entry.currentValue = entry.lastValidValue;
    }

    if (entry.currentValue == "") {
      entry.currentValue = entry.placeholder;
    }
  }

  if (entry.entrySelected) {
    Input::CharacterData data = Input::getCharacters();

    for (uint64_t i = 0; i < data.size; i++) {
      uint32_t codepoint = data.codepoints[i];

      char character = static_cast<char>(codepoint);

      if (isprint(character)) {
        // TODO: right now we jsut pretend every codepoint is a valid ASCII
        // character We want to only consider numbers
        // TODO: deal with negatives as well

        if (isdigit(character) || character == '-') {
          entry.currentValue += character;
        }

        VIVIUM_LOG(LogSeverity::DEBUG, "Printable but not digit: {}",
                   codepoint);
      } else {
        VIVIUM_LOG(LogSeverity::WARN,
                   "Received unprintable character codepoint {}", codepoint);
      }
    }
  }

  // TODO: only update text if change was made
  setButtonText(entry.inputArea, engine, context, guiContext,
                entry.currentValue);
}

void updateEntry(FloatTextEntry& entry, GUIContext& guiContext, Engine& engine,
                 CommandContext& context) {
  bool clicked = (Input::get(Input::BTN_1).state == Input::RELEASE);
  bool hovered =
      pointInElement(Input::getCursor(), properties(entry.base, guiContext));
  bool entered = (Input::get(Input::KEY_ENTER).state == Input::RELEASE);

  // TODO: deal with backspace and all the text handling stuff
  //	should only enter character if cursor hovering/clicked/selected
  if (clicked && hovered) {
    entry.entrySelected = true;
    entry.currentValue = "";
  }

  if ((clicked && !hovered) || entered) {
    entry.entrySelected = false;

    // Float
    std::regex floatPattern("-?\\d*(\\.\\d+)?");

    if (std::regex_match(entry.currentValue, floatPattern)) {
      entry.lastValidValue = entry.currentValue;
    } else {
      VIVIUM_LOG(LogSeverity::WARN, "Invalid input entered: {}",
                 entry.currentValue);

      entry.currentValue = entry.lastValidValue;
    }

    if (entry.currentValue == "") {
      entry.currentValue = entry.placeholder;
    }
  }

  if (entry.entrySelected) {
    Input::CharacterData data = Input::getCharacters();

    for (uint64_t i = 0; i < data.size; i++) {
      uint32_t codepoint = data.codepoints[i];

      char character = static_cast<char>(codepoint);

      if (isprint(character)) {
        // TODO: right now we jsut pretend every codepoint is a valid ASCII
        // character We want to only consider numbers
        // TODO: if it would lead to invalid currentValue, don't allow it

        if (isdigit(character) || character == '.' || character == '-') {
          entry.currentValue += character;
        }

        VIVIUM_LOG(LogSeverity::DEBUG, "Printable but not digit: {}",
                   codepoint);
      } else {
        VIVIUM_LOG(LogSeverity::WARN,
                   "Received unprintable character codepoint {}", codepoint);
      }
    }
  }

  // TODO: only update text if change was made
  setButtonText(entry.inputArea, engine, context, guiContext,
                entry.currentValue);
}

void updateEntry(StringTextEntry& entry, GUIContext& guiContext, Engine& engine,
                 CommandContext& context) {
  bool clicked = (Input::get(Input::BTN_1).state == Input::RELEASE);
  bool hovered =
      pointInElement(Input::getCursor(), properties(entry.base, guiContext));
  bool entered = (Input::get(Input::KEY_ENTER).state == Input::RELEASE);

  // TODO: deal with backspace and all the text handling stuff
  //	should only enter character if cursor hovering/clicked/selected
  if (clicked && hovered) {
    entry.entrySelected = true;
    entry.currentValue = "";
  }

  if ((clicked && !hovered) || entered) {
    entry.entrySelected = false;

    entry.lastValidValue = entry.currentValue;

    if (entry.currentValue == "") {
      entry.currentValue = entry.placeholder;
    }
  }

  if (entry.entrySelected) {
    Input::CharacterData data = Input::getCharacters();

    for (uint64_t i = 0; i < data.size; i++) {
      uint32_t codepoint = data.codepoints[i];

      char character = static_cast<char>(codepoint);

      if (isprint(character)) {
        entry.currentValue += character;
      } else {
        VIVIUM_LOG(LogSeverity::WARN,
                   "Received unprintable character codepoint {}", codepoint);
      }
    }
  }

  // TODO: only update text if change was made
  setButtonText(entry.inputArea, engine, context, guiContext,
                entry.currentValue);
}

void submitEntries(std::span<IntegerTextEntry*> const entries,
                   GUIContext& context) {
  for (IntegerTextEntry* entry : entries) {
    Button* entryList[] = {&entry->inputArea};
    submitButtons(entryList, context);
  }
}

void submitEntries(std::span<FloatTextEntry*> const entries,
                   GUIContext& context) {
  for (FloatTextEntry* entry : entries) {
    Button* entryList[] = {&entry->inputArea};
    submitButtons(entryList, context);
  }
}

void submitEntries(std::span<StringTextEntry*> const entries,
                   GUIContext& context) {
  for (StringTextEntry* entry : entries) {
    Button* entryList[] = {&entry->inputArea};
    submitButtons(entryList, context);
  }
}

void dropEntry(IntegerTextEntry& entry, Engine& engine,
               GUIContext& guiContext) {
  dropButton(entry.inputArea, engine, guiContext);
}

void dropEntry(FloatTextEntry& entry, Engine& engine, GUIContext& guiContext) {
  dropButton(entry.inputArea, engine, guiContext);
}

void dropEntry(StringTextEntry& entry, Engine& engine, GUIContext& guiContext) {
  dropButton(entry.inputArea, engine, guiContext);
}

int getValue(IntegerTextEntry const& entry) { return stoi(entry.currentValue); }

float getValue(FloatTextEntry const& entry) { return stof(entry.currentValue); }

std::string getValue(StringTextEntry const& entry) {
  return entry.currentValue;
}

void loadValue(IntegerTextEntry& entry, int value, GUIContext& guiContext) {
  entry.currentValue = std::to_string(value);
  entry.lastValidValue = entry.currentValue;
}

void loadValue(FloatTextEntry& entry, float value, GUIContext& guiContext) {
  entry.currentValue = std::to_string(value);
  entry.lastValidValue = entry.currentValue;
}

void loadValue(StringTextEntry& entry, std::string const& value,
               GUIContext& guiContext) {
  entry.currentValue = value;
  entry.lastValidValue = value;
}
}  // namespace Vivium
