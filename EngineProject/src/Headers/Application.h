#ifndef application_h
#define application_h
#include <windows.h>


#include "MainVulkan.h"
#include <iostream>


//�������� �����
class SceneEditor{
private:
	
	bool DemoWindowActive    = true,  //���� ImGui (��� �������)
		 ShowHierarchyPanel  = true,  //�������� ������ ��������
		 ShowPropertiesPanel = true,  //�������� ������ �������
		 StartButtonActive,			  //��������� ������� ������ �����
		 ResetPhysics        = true,  //�������� ���������� ��������� (���� ��������� �� ������)
		 CloseWindow         = false, //������� ����
		 OpenFileDialog		 = false; //����� ����������� ���� ��� �������� �����
	
public:
	std::atomic<bool> LoadingIsEnded = true;//�������� ���������� ������ ��������
	bool enableEditor = true;
	Engine::EditorCamera editorCamera{};//������ ��������� ����
	int   SelectedItem_ID  = -1;  //������� ��������� �������
	float MenubarHeight = 0;    //������ �������� (��� ���������� ��������)
public:

	//��������� ImGui
	void InitEditor(HWND hwnd);

	//��������� ���������� � ������� �����
	void DrawEditor(HWND hwnd, std::vector<Engine::Entity*>& Entities);
	
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);



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
private:
	float		FpsCapTimer = 0;
	
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