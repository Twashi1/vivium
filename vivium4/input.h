#pragma once

#include <array>

#include "core.h"
#include "window.h"
#include "storage.h"

/*
Core API;

Input::Context::Handle handle = Input::Context::create(storage, windowHandle);
Input::isDown(handle, Input::Mouse::LMB) -> bool
*/

namespace Vivium {
	constexpr uint64_t MAX_CHARACTER_INPUTS_PER_FRAME = 64;

	class Input {
	public:
		enum State : uint8_t {
			RELEASE		= 0b0000'0000,
			UP			= 0b0000'0010,
			DOWN		= 0b0000'0011,
			PRESS		= 0b0000'0001,

			IS_HELD_MASK	= 0b0000'0010,
			IS_PRESSED_MASK = 0b0000'0001
		};

		enum Modifier : uint8_t {
			MOD_NONE		= 0,
			MOD_SHIFT		= GLFW_MOD_SHIFT,
			MOD_CONTROL		= GLFW_MOD_CONTROL,
			MOD_ALT			= GLFW_MOD_ALT,
			MOD_SUPER		= GLFW_MOD_SUPER,
			MOD_CAPS_LOCK	= GLFW_MOD_CAPS_LOCK,
			MOD_NUM_LOCK	= GLFW_MOD_NUM_LOCK,
			_MOD_MIN		= MOD_SHIFT,
			_MOD_MAX		= MOD_NUM_LOCK
		};

		enum Key : int {
			// Copied straight from glfw
			KEY_SPACE = 32,
			KEY_APOSTROPHE = 39,  /* ' */
			KEY_COMMA = 44,  /* , */
			KEY_MINUS = 45,  /* - */
			KEY_PERIOD = 46,  /* . */
			KEY_SLASH = 47,  /* / */
			KEY_0 = 48,
			KEY_1 = 49,
			KEY_2 = 50,
			KEY_3 = 51,
			KEY_4 = 52,
			KEY_5 = 53,
			KEY_6 = 54,
			KEY_7 = 55,
			KEY_8 = 56,
			KEY_9 = 57,
			KEY_SEMICOLON = 59,  /* ; */
			KEY_EQUAL = 61,  /* = */
			KEY_A = 65,
			KEY_B = 66,
			KEY_C = 67,
			KEY_D = 68,
			KEY_E = 69,
			KEY_F = 70,
			KEY_G = 71,
			KEY_H = 72,
			KEY_I = 73,
			KEY_J = 74,
			KEY_K = 75,
			KEY_L = 76,
			KEY_M = 77,
			KEY_N = 78,
			KEY_O = 79,
			KEY_P = 80,
			KEY_Q = 81,
			KEY_R = 82,
			KEY_S = 83,
			KEY_T = 84,
			KEY_U = 85,
			KEY_V = 86,
			KEY_W = 87,
			KEY_X = 88,
			KEY_Y = 89,
			KEY_Z = 90,
			KEY_LEFT_BRACKET = 91,  /* [ */
			KEY_BACKSLASH = 92,  /* \ */
			KEY_RIGHT_BRACKET = 93,  /* ] */
			KEY_GRAVE_ACCENT = 96,  /* ` */
			KEY_WORLD_1 = 161, /* non-US #1 */
			KEY_WORLD_2 = 162, /* non-US #2 */
			KEY_ESCAPE = 256,
			KEY_ENTER = 257,
			KEY_TAB = 258,
			KEY_BACKSPACE = 259,
			KEY_INSERT = 260,
			KEY_DELETE = 261,
			KEY_RIGHT = 262,
			KEY_LEFT = 263,
			KEY_DOWN = 264,
			KEY_UP = 265,
			KEY_PAGE_UP = 266,
			KEY_PAGE_DOWN = 267,
			KEY_HOME = 268,
			KEY_END = 269,
			KEY_CAPS_LOCK = 280,
			KEY_SCROLL_LOCK = 281,
			KEY_NUM_LOCK = 282,
			KEY_PRINT_SCREEN = 283,
			KEY_PAUSE = 284,
			KEY_F1 = 290,
			KEY_F2 = 291,
			KEY_F3 = 292,
			KEY_F4 = 293,
			KEY_F5 = 294,
			KEY_F6 = 295,
			KEY_F7 = 296,
			KEY_F8 = 297,
			KEY_F9 = 298,
			KEY_F10 = 299,
			KEY_F11 = 300,
			KEY_F12 = 301,
			KEY_F13 = 302,
			KEY_F14 = 303,
			KEY_F15 = 304,
			KEY_F16 = 305,
			KEY_F17 = 306,
			KEY_F18 = 307,
			KEY_F19 = 308,
			KEY_F20 = 309,
			KEY_F21 = 310,
			KEY_F22 = 311,
			KEY_F23 = 312,
			KEY_F24 = 313,
			KEY_F25 = 314,
			KEY_KP_0 = 320,
			KEY_KP_1 = 321,
			KEY_KP_2 = 322,
			KEY_KP_3 = 323,
			KEY_KP_4 = 324,
			KEY_KP_5 = 325,
			KEY_KP_6 = 326,
			KEY_KP_7 = 327,
			KEY_KP_8 = 328,
			KEY_KP_9 = 329,
			KEY_KP_DECIMAL = 330,
			KEY_KP_DIVIDE = 331,
			KEY_KP_MULTIPLY = 332,
			KEY_KP_SUBTRACT = 333,
			KEY_KP_ADD = 334,
			KEY_KP_ENTER = 335,
			KEY_KP_EQUAL = 336,
			KEY_LEFT_SHIFT = 340,
			KEY_LEFT_CONTROL = 341,
			KEY_LEFT_ALT = 342,
			KEY_LEFT_SUPER = 343,
			KEY_RIGHT_SHIFT = 344,
			KEY_RIGHT_CONTROL = 345,
			KEY_RIGHT_ALT = 346,
			KEY_RIGHT_SUPER = 347,
			KEY_MENU = 348,

			_KEY_MIN = KEY_SPACE,
			_KEY_MAX = KEY_MENU,
			_KEY_SIZE = _KEY_MAX - _KEY_MIN + 1
		};

		enum Button : int {
			BTN_LEFT	= GLFW_MOUSE_BUTTON_LEFT,
			BTN_MIDDLE	= GLFW_MOUSE_BUTTON_MIDDLE,
			BTN_RIGHT	= GLFW_MOUSE_BUTTON_RIGHT,
			BTN_1		= GLFW_MOUSE_BUTTON_1, // Same as left
			BTN_2		= GLFW_MOUSE_BUTTON_2, // Same as middle
			BTN_3		= GLFW_MOUSE_BUTTON_3, // Same as right
			BTN_4		= GLFW_MOUSE_BUTTON_4,
			BTN_5		= GLFW_MOUSE_BUTTON_5,
			BTN_6		= GLFW_MOUSE_BUTTON_6,
			BTN_7		= GLFW_MOUSE_BUTTON_7,
			BTN_8		= GLFW_MOUSE_BUTTON_8,

			_BTN_MIN = BTN_1,
			_BTN_MAX = BTN_8,
			_BTN_SIZE = _BTN_MAX - _BTN_MIN + 1
		};

		struct Listener {
			State state;
			float timeHeld;

			operator bool() const { return state & State::IS_PRESSED_MASK; }

			Listener();
		};

		struct CharacterData {
			std::array<uint32_t, MAX_CHARACTER_INPUTS_PER_FRAME> codepoints;
			uint64_t size;
		};

	private:
		static std::array<Listener, _KEY_MAX> m_keyListeners;
		static std::array<Listener, _BTN_MAX> m_buttonListeners;

		static Time::Timer m_holdTimer;

		static CharacterData m_frontCharacters, m_backCharacters;

		static Modifier m_currentModifiers;

		static F32x2 m_cursorPosition;

		static void m_updateListener(Listener& listener, int action, float deltaTime);
		static void m_characterInputCallback(GLFWwindow* window, uint32_t codepoint);

	public:
		static Listener get(Key key);
		static Listener get(Button button);

		static void update(Window::Handle window);

		static void init(Window::Handle window);

		static Modifier getModifiers();
		static F32x2 getCursor();
	};
}