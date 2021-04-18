#ifndef application_h
#define application_h
#include <windows.h>


#include "MainVulkan.h"
#include <iostream>
#include "Renderer/Renderer.h"

//�������� �����
class SceneEditor{
public:
	
	bool DemoWindowActive    = true,  //���� ImGui (��� �������)
		 ShowHierarchyPanel  = true,  //�������� ������ ��������
		 ShowPropertiesPanel = true,  //�������� ������ �������
		 StartButtonActive,			  //��������� ������� ������ �����
		 ResetPhysics        = true,  //�������� ���������� ��������� (���� ��������� �� ������)
		 CloseWindow         = false; //������� ����

public:
	Engine::EditorCamera editorCamera{};
	int   SelectedItem_ID  = -1;  //������� ��������� �������
	float MenubarHeight = 0;    //������ �������� (��� ���������� ��������)
public:

	//�������������� ��������� ImGui
	void InitEditor(HWND hwnd);

	//��������� ���������� � ������� �����
	void DrawEditor(HWND hwnd, const std::vector<Engine::Entity*>* Entities);
	
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	//��������� ������� ������� � ImGui
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return true;
	
		switch (msg) {
			case WM_SYSKEYDOWN:

				if ((HIWORD(lparam) & 0x4000) == 0) {
					Engine::Globals::keyPressedEventHandler.ProcessKeyDownEvent(wparam, lparam);
				}

				MSG m;
				m = {};
				m.hwnd = hwnd;
				m.message = msg;

				PeekMessage(&m, NULL, WM_SYSCHAR, WM_SYSCHAR, PM_REMOVE);

				return ::DefWindowProc(hwnd, msg, wparam, lparam);
				break;

			case WM_SYSKEYUP:
				Engine::Globals::keyPressedEventHandler.ProcessKeyUpEvent(wparam, lparam);
				return true;
				break;

			case WM_KEYDOWN:
				if ((HIWORD(lparam) & 0x4000) == 0){
					Engine::Globals::keyPressedEventHandler.ProcessKeyDownEvent(wparam, lparam);
				}

				return true;
				break;

			case WM_CHAR:
				if (wparam == VK_RETURN){
					return 0;
				}
				break;

			case WM_KEYUP:
				Engine::Globals::keyPressedEventHandler.ProcessKeyUpEvent(wparam, lparam);
				return true;
				break;

			case WM_SIZE:
				//std::cout << "CALL TO WM_SIZE" << wparam << std::endl;

				if (Engine::renderer.device.Get() != VK_NULL_HANDLE){
					Engine::renderer.recreateSwapchain();
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
		
	}
	return ::DefWindowProc(hwnd, msg, wparam, lparam);
}



class Application {
public:
	SceneEditor sceneEditor;

	HWND	    hwnd;
	HINSTANCE   hInstance;
	WNDCLASSEX	wc;

	

	ImDrawData* ImguiDrawData;

	int			WindowWidth, 
				WindowHeight;

    double		Time,  
				LastFrameTime;
	
	//json		settingsJson;
public:
	//���������� ����������
	void Init();

	//������� ����
	void Execute();

	//������������ ��������
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