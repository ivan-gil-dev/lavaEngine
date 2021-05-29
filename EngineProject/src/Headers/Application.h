#ifndef application_h
#define application_h
#include <windows.h>


#include "MainVulkan.h"
#include <iostream>


//Редактор сцены
class SceneEditor{
private:
	
	bool DemoWindowActive    = true,  //Демо ImGui (для отладки)
		 ShowHierarchyPanel  = true,  //Показать панель иерархии
		 ShowPropertiesPanel = true,  //Показать панель свойств
		 StartButtonActive,			  //Обработка нажатия кнопки старт
		 ResetPhysics        = true,  //Сбросить физические параметры (Если симуляция не начата)
		 CloseWindow         = false, //Закрыть окно
		 OpenFileDialog		 = false; //Вывод диалогового окна для открытия файла
	
public:
	std::atomic<bool> LoadingIsEnded = true;//Ожидание завершения потока загрузки
	bool enableEditor = true;
	Engine::EditorCamera editorCamera{};//Камера редактора сцен
	int   SelectedItem_ID  = -1;  //Текущий выбранный элемент
	float MenubarHeight = 0;    //Высота менюбара (для вычисления отступов)
public:

	//настройка ImGui
	void InitEditor(HWND hwnd);

	//Обработка интерфейса в главном цикле
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
	//Подготовка приложения
	void Init();

	//Главный цикл
	void Execute();

	//Освобождение ресурсов
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