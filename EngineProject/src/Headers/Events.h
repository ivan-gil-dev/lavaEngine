#ifndef events_h
#define events_h


#include "Globals.h"

namespace Engine{

	//<Перечисление кодов клавиш>
	enum ENGINE_KEYS{
		KEY_A = 0x41,
		KEY_B = 0x42,
		KEY_C = 0x43,
		KEY_D = 0x44,
		KEY_E = 0x45,
		KEY_F = 0x46,
		KEY_G = 0x47,
		KEY_H = 0x48,
		KEY_I = 0x49,
		KEY_J = 0x4A,
		KEY_K = 0x4B,
		KEY_L = 0x4C,
		KEY_M = 0x4D,
		KEY_N = 0x4E,
		KEY_O = 0x4F,
		KEY_P = 0x50,
		KEY_Q = 0x51,
		KEY_R = 0x52,
		KEY_S = 0x53,
		KEY_T = 0x54,
		KEY_U = 0x55,
		KEY_V = 0x56,
		KEY_W = 0x57,
		KEY_X = 0x58,
		KEY_Y = 0x59,
		KEY_Z = 0x60,
		KEY_ALT = VK_MENU,
		KEY_SHIFT = VK_SHIFT,
		KEY_CONTROL = VK_CONTROL
	};

	//<Обработчик нажатия клавиши>
	class KeyPressedEventHandler{

		std::vector<ENGINE_KEYS> pressedKeys;
		public:

		void ProcessKeyDownEvent(WPARAM key,LPARAM param){
			for (size_t i = 0; i < pressedKeys.size(); i++) {
				if (pressedKeys[i] == key) {
					return;
				}
			}
			pressedKeys.push_back(ENGINE_KEYS(key));
		}

		void ProcessKeyUpEvent(WPARAM key, LPARAM param) {
			for (size_t i = 0; i < pressedKeys.size(); i++) {
				if (pressedKeys[i] == key) {
					pressedKeys.erase(pressedKeys.begin()+i);
					return;
				}
			}
		}

		bool IsKeyPressed(ENGINE_KEYS key){
			for (size_t i = 0; i < pressedKeys.size(); i++) {
				if (pressedKeys[i] == key){
					return true;
				}
			}
			return false;
		}
	
	};

	//<Обработчик движения мышью>
	class MouseMoveEventHandler{
	glm::vec2 cursorPos;
	public:
		void SetCursorPos(double X, double Y){
			cursorPos.x = float(X);
			cursorPos.y = float(Y);
		}

		glm::vec2 GetCursorPos(){
			return cursorPos;
		}

	};

	//<Обработчик вывода курсора>
	class ShowCursorEventHandler {
		bool IsShowing = true;
	public:
		void ShowCursor(){
			if (!IsShowing){
				::ShowCursor(true);
				IsShowing = true;
			}
		}
		void HideCursor(){
			if (IsShowing){
				::ShowCursor(false);
				IsShowing = false;
			}
		}

	};

	namespace Globals{
		extern KeyPressedEventHandler keyPressedEventHandler;
		extern MouseMoveEventHandler  mouseMoveEventHandler;
		extern ShowCursorEventHandler showCursorEventHandler;
	}
	
}

#endif