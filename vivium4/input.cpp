#include "input.h"

namespace Vivium {
	std::array<Input::Listener, Input::_KEY_MAX> Input::m_keyListeners;
	std::array<Input::Listener, Input::_BTN_MAX> Input::m_buttonListeners;
	
	Time::Timer Input::m_holdTimer;
	
	Input::CharacterData Input::m_frontCharacters, Input::m_backCharacters;
	Input::Modifier Input::m_currentModifiers;

	F32x2 Input::m_cursorPosition;
	
	void Input::m_updateListener(Listener& listener, int action, float deltaTime)
	{
		listener.timeHeld += deltaTime;

		State converted;

		switch (action) {
		case GLFW_REPEAT:
		case GLFW_PRESS:
			converted = State::PRESS; break;
		case GLFW_RELEASE:
			converted = State::RELEASE; break;
		}

		if (converted == (listener.state & State::IS_PRESSED_MASK))
			listener.state = State(converted | State::IS_HELD_MASK);
		else
		{
			listener.state = converted;
			listener.timeHeld = 0.0f;
		}
	}

	void Input::m_characterInputCallback(GLFWwindow* window, uint32_t codepoint)
	{
		m_backCharacters.codepoints[m_backCharacters.size++] = codepoint;
	}

	Input::Listener Input::get(Key key)
	{
		return m_keyListeners[key];
	}

	Input::Listener Input::get(Button button)
	{
		return m_buttonListeners[button];
	}

	void Input::update(Window::Handle window)
	{
		float deltaTime = m_holdTimer.reset();

		double cursorX, cursorY;
		glfwGetCursorPos(window->glfwWindow, &cursorX, &cursorY);

		// Flip y
		m_cursorPosition = F32x2(cursorX, window->dimensions.y - cursorY);

		for (int i = _KEY_MIN; i < _KEY_MAX; i++) {
			m_updateListener(m_keyListeners[i], glfwGetKey(window->glfwWindow, i), deltaTime);
		}

		for (int i = _BTN_MIN; i < _BTN_MAX; i++) {
			m_updateListener(m_buttonListeners[i], glfwGetMouseButton(window->glfwWindow, i), deltaTime);
		}

		// Swap front and back character input
		std::swap(m_frontCharacters.codepoints, m_backCharacters.codepoints);
		m_frontCharacters.size = std::exchange(m_backCharacters.size, 0);

		// Update modifiers
		m_currentModifiers = static_cast<Modifier>(0);

		// TODO: really need to be more efficient here
		if ((m_keyListeners[KEY_LEFT_ALT].state & IS_PRESSED_MASK)
			|| (m_keyListeners[KEY_RIGHT_ALT].state & IS_PRESSED_MASK)) {
			m_currentModifiers = static_cast<Modifier>(m_currentModifiers | MOD_ALT);
		}

		if ((m_keyListeners[KEY_LEFT_SHIFT].state & IS_PRESSED_MASK)
			|| (m_keyListeners[KEY_RIGHT_SHIFT].state & IS_PRESSED_MASK)) {
			m_currentModifiers = static_cast<Modifier>(m_currentModifiers | MOD_SHIFT);
		}

		if ((m_keyListeners[KEY_LEFT_CONTROL].state & IS_PRESSED_MASK)
			|| (m_keyListeners[KEY_RIGHT_CONTROL].state & IS_PRESSED_MASK)) {
			m_currentModifiers = static_cast<Modifier>(m_currentModifiers | MOD_CONTROL);
		}

		if ((m_keyListeners[KEY_LEFT_SUPER].state & IS_PRESSED_MASK)
			|| (m_keyListeners[KEY_RIGHT_SUPER].state & IS_PRESSED_MASK)) {
			m_currentModifiers = static_cast<Modifier>(m_currentModifiers | MOD_SUPER);
		}

		if (m_keyListeners[KEY_CAPS_LOCK].state & IS_PRESSED_MASK) {
			m_currentModifiers = static_cast<Modifier>(m_currentModifiers | MOD_CAPS_LOCK);
		}

		if (m_keyListeners[KEY_NUM_LOCK].state & IS_PRESSED_MASK) {
			m_currentModifiers = static_cast<Modifier>(m_currentModifiers | MOD_NUM_LOCK);
		}
	}

	void Input::init(Window::Handle window)
	{
		glfwSetCharCallback(window->glfwWindow, m_characterInputCallback);
	
		m_holdTimer.reset();
	}

	Input::Modifier Input::getModifiers()
	{
		return m_currentModifiers;
	}

	F32x2 Input::getCursor()
	{
		return m_cursorPosition;
	}
	
	Input::Listener::operator bool() const
	{
		return state & State::IS_PRESSED_MASK;
	}

	Input::Listener::Listener()
		: state(State::UP), timeHeld(0.0f)
	{}
}