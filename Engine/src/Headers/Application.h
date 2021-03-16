#ifndef application_h
#define application_h

#include "MainVulkan.h"
#include <iostream>

// Редактор сцены
class SceneEditor{
public:
	
	bool  DemoWindowActive = true,// отображение демо ImGui
		  ShowHierarchyPanel = true,// упр. панелью иерархии
		  ShowPropertiesPanel = true,// упр. панелью свойств
		  StartButtonActive,// упр. кнопкой старт
		  ResetPhysics = true,// сброс физической модели
		  CloseWindow = false;// Выход из приложения

public:
	int   SelectedItem_ID  = 1 ; //Выбранный объект из меню иерархии
	float MenubarHeight = 0 ;
public:

	// Настройка ImGui
	void InitEditor(HWND hwnd);

	// Запись команд для вывода ImGui
	void DrawEditor(HWND hwnd, const std::vector<Engine::Entity*>& Entities);
	
};

static SceneEditor sceneEditor;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	/* Обработка оконных забытий в ImGui
	*/
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return true;

	{
		switch (msg) {
		case WM_SYSKEYDOWN:
			if (wparam == VK_RETURN){
				MSG msg2 = { };
				PeekMessage(&msg2, hwnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE);
			}
			Engine::Globals::keyPressedEventHandler.ProcessKeyDownEvent(wparam, lparam);
			return true;
			break;

		case WM_SYSKEYUP:
			Engine::Globals::keyPressedEventHandler.ProcessKeyUpEvent(wparam, lparam);
			return true;
			break;

		case WM_KEYDOWN:
			Engine::Globals::keyPressedEventHandler.ProcessKeyDownEvent(wparam, lparam);
			return true;
			break;

		case WM_KEYUP:
			Engine::Globals::keyPressedEventHandler.ProcessKeyUpEvent(wparam, lparam);
			return true;
			break;

		case WM_SIZE:
			if (Engine::Globals::gDevice.Get() != VK_NULL_HANDLE){
				Engine::recreateSwapchain();
			}
			return true;
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			break;

		case WM_MOUSEMOVE:
			Engine::Globals::mouseMoveEventHandler.SetCursorPos(LOWORD(lparam), HIWORD(lparam));
			return true;
			break;

		default:
			return ::DefWindowProc(hwnd, msg, wparam, lparam);
			break;
		}
	}
}



class Application {
public:
	HWND		hwnd;
	HINSTANCE   hInstance;
	WNDCLASSEX	wc;
	ImDrawData* ImguiDrawData;
	int			WindowWidth, 
				WindowHeight;
	double		Time,  
				LastFrameTime;
public:
	/* Настройка приложения
	*/
	void Init();

	/* Главный цикл
	*/
	void Execute();

	/* Освобождение ресурсов
	*/
	void Clear();

	uint32_t GetWidth();

	uint32_t GetHeight();
};

namespace Engine{
	namespace Globals{
		extern Application App;
	}
}

#endif