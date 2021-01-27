#pragma once
#include "Globals.h"

namespace Lava{
	enum LAVA_KEYS{
		LAVA_KEY_A = 0x41,
		LAVA_KEY_B = 0x42,
		LAVA_KEY_C = 0x43,
		LAVA_KEY_D = 0x44,
		LAVA_KEY_E = 0x45,
		LAVA_KEY_F = 0x46,
		LAVA_KEY_G = 0x47,
		LAVA_KEY_H = 0x48,
		LAVA_KEY_I = 0x49,
		LAVA_KEY_J = 0x4A,
		LAVA_KEY_K = 0x4B,
		LAVA_KEY_L = 0x4C,
		LAVA_KEY_M = 0x4D,
		LAVA_KEY_N = 0x4E,
		LAVA_KEY_O = 0x4F,
		LAVA_KEY_P = 0x50,
		LAVA_KEY_Q = 0x51,
		LAVA_KEY_R = 0x52,
		LAVA_KEY_S = 0x53,
		LAVA_KEY_T = 0x54,
		LAVA_KEY_U = 0x55,
		LAVA_KEY_V = 0x56,
		LAVA_KEY_W = 0x57,
		LAVA_KEY_X = 0x58,
		LAVA_KEY_Y = 0x59,
		LAVA_KEY_Z = 0x60,
	};

	

	class KeyPressedEventHandler{
		std::vector<LAVA_KEYS> pressedKeys;
		public:
		
		void ProcessKeyDownEvent(WPARAM key){
			for (size_t i = 0; i < pressedKeys.size(); i++) {
				if (pressedKeys[i] == key) {
					return;
				}
			}
			pressedKeys.push_back(LAVA_KEYS(key));
		}

		void ProcessKeyUpEvent(WPARAM key) {
			for (size_t i = 0; i < pressedKeys.size(); i++) {
				if (pressedKeys[i] == key) {
					pressedKeys.erase(pressedKeys.begin()+i);
					return;
				}
			}
		}

		bool IsKeyPressed(LAVA_KEYS key){
			for (size_t i = 0; i < pressedKeys.size(); i++) {
				if (pressedKeys[i] == key){
					return true;
				}
			}
			return false;
		}
	
	}keyPressedEventHandler;

	class MouseMoveEventHandler{
	glm::vec2 cursorPos;
	public:
		void SetCursorPos(double X, double Y){
			cursorPos.x = X;
			cursorPos.y = Y;
		}
		glm::vec2 GetCursorPos(){
			return cursorPos;
		}

	}mouseMoveEventHandler;

	class ShowCursorEventHandler {
	
		bool IsShowing = true;
	public:
		void ShowCursor(){
			if (!IsShowing)
			{
				::ShowCursor(true);
				IsShowing = true;
			}
		}
		void HideCursor(){
			if (IsShowing)
			{
				::ShowCursor(false);
				IsShowing = false;
			}
		}

	}showCursorEventHandler;

}

