#pragma once

#include "MainVulkan.h"
#include <iostream>

// Редактор сцены
class SceneEditor{
	bool  DemoWindowActive = true,
		  StartButtonActive,      
		  ResetPhysics = true,
		  CloseWindowMenuItem = false ;

	int   SelectedItem  = -1 ; //: Выбранный объект из меню иерархии
	float MenubarHeight = 0 ;
public:

	//: Инициализация объектов ImGui
	void InitEditor(HWND hwnd){
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsClassic();
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.WindowPadding = ImVec2(2, 2);
		style.FramePadding = ImVec2(2, 2);
		style.WindowTitleAlign = ImVec2(0.5, 0.5);

		style.WindowMenuButtonPosition = ImGuiDir_Right;
		// Setup Platform/Renderer bindings
		if (ImGui_ImplWin32_Init(hwnd) != true) {
			throw std::runtime_error("Failed to init imguiWin32 for vulkan");
		}
		
		//: Передача компонентов Vulkan в ImGui
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance =	   Lava::gInstance.get();
		init_info.PhysicalDevice = Lava::gPhysicalDevice.Get();
		init_info.Device =		  *Lava::gDevice.PGet();
		init_info.QueueFamily =	   Lava::gPhysicalDevice.GetQueueIndices().graphicsQueueIndex;
		init_info.Queue =		   Lava::gDevice.GetGraphicsQueue();
		init_info.PipelineCache =  nullptr;
		init_info.DescriptorPool = Lava::gDescriptorPoolForImgui._Get_();
		init_info.Allocator =      nullptr;
		init_info.MinImageCount =  Lava::gSwapchain.GetInfo().minImageCount;
		init_info.ImageCount =     Lava::gSwapchain.GetInfo().minImageCount;
		init_info.CheckVkResultFn = imguiErrFunction;

		ImGui_ImplVulkan_Init(&init_info, Lava::gRenderPass.GetRenderPass());

		//: Загрузка шрифтов в GPU
		Lava::CommandBuffer oneTimeSubmitCommandBuffer;
		oneTimeSubmitCommandBuffer.AllocateCommandBuffer(Lava::gDevice.Get(), 
		Lava::gCommandPool.Get());
		oneTimeSubmitCommandBuffer.BeginCommandBuffer();

		ImGui_ImplVulkan_CreateFontsTexture(
			oneTimeSubmitCommandBuffer.Get()
		);

		oneTimeSubmitCommandBuffer.EndCommandBuffer();
		oneTimeSubmitCommandBuffer.SubmitCommandBuffer(Lava::gDevice.GetGraphicsQueue());
		oneTimeSubmitCommandBuffer.FreeCommandBuffer(Lava::gDevice.Get(), 
															Lava::gCommandPool.Get());

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	//: Запись команд для отрисовки элементов интерфейса
	void PrepareToRender(HWND hwnd, const std::vector<Lava::Entity*>& GameObjects, bool& sceneIsPlaying) {
		if (LAVA_GLOBAL_ENABLE_IMGUI) {

			//: Show demo window
			ImGui::ShowDemoWindow(&DemoWindowActive); 

			//: Увеличить менюбар
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 20));
			
			//: Начало записи команд для менюбара
			ImGui::BeginMainMenuBar();

			ImGui::PopStyleVar();
			MenubarHeight = ImGui::GetWindowHeight();

			if (ImGui::BeginMenu(u8"File")) {
				ImGui::MenuItem(u8"Exit", "ALT + F4", &CloseWindowMenuItem);
				ImGui::EndMenu();
				if (CloseWindowMenuItem) {
					SendMessage(hwnd,WM_CLOSE,0,0);
				}
			}

			if (ImGui::BeginMenu(u8"Simulation")) {
				ImGui::MenuItem(u8"Start/Stop", "", &StartButtonActive);
				
				if (StartButtonActive) {
					sceneIsPlaying = true; 
					ResetPhysics = true;
					
					//: Растягивание вьюпорта по всей поверхности отображения
					Lava::gEditor3DViewport.x = 0;
					Lava::gEditor3DViewport.y = 0;
					Lava::gEditor3DViewport.height = Lava::gSwapchain.GetInfo().imageExtent.height;
					Lava::gEditor3DViewport.width = Lava::gSwapchain.GetInfo().imageExtent.width;
					Lava::gEditor3DScissors.extent = Lava::gSwapchain.GetInfo().imageExtent;

				}
				else {
					sceneIsPlaying = false; 
					if (ResetPhysics) {
						for (size_t i = 0; i < GameObjects.size(); i++) {
							//: Сброс физических свойств, трансформаций объектов
							if (GameObjects[i]->GetType() == Lava::LAVA_ENTITY_TYPE_GAME_OBJECT) {
								GameObjects[i]->Transform.ResetTransform();
								GameObjects[i]->ApplyEntityTransformToRigidbodyAndMesh();
								if (GameObjects[i]->GetRigidBody()->GetBulletRigidBody() != nullptr) {
									GameObjects[i]->GetRigidBody()->GetBulletRigidBody()->clearForces();
									GameObjects[i]->GetRigidBody()->GetBulletRigidBody()->clearGravity();
									GameObjects[i]->GetRigidBody()->GetBulletRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
									GameObjects[i]->GetRigidBody()->GetBulletRigidBody()->setAngularVelocity(btVector3(0, 0, 0));
								}
							}
						}
						Lava::gDynamicsWorld->clearForces();
						Lava::gSolver->reset();
						ResetPhysics = false; //: Предотвращение повторного вызова цикла выше
					}
				}
				ImGui::EndMenu();
			}


			if (ImGui::BeginMenu(u8"View")) {
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();

			if (!sceneIsPlaying) {

				//: Панель иерархии
				ImGui::Begin(u8"Hierarchy Panel", NULL, ImGuiWindowFlags_NoMove);

				//: Масштабирование вьюпорта в зависимости от размера 
				//  панели иерархии
				Lava::gEditor3DViewport.x = ImGui::GetWindowWidth();
				Lava::gEditor3DViewport.y = MenubarHeight;
 				Lava::gEditor3DViewport.width = Lava::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowWidth();
				Lava::gEditor3DViewport.height = Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight;
				Lava::gEditor3DScissors.extent = Lava::gSwapchain.GetInfo().imageExtent;

				//: Фиксирование положения панели иерархии
				ImGui::SetWindowPos(ImVec2(0, MenubarHeight)); 
				ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(),
					Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight)); // Растянуть по высоте окна
				
				//: Вывод списка объектов
				for (size_t i = 0; i < GameObjects.size(); i++) {
					if (ImGui::TreeNode(GameObjects[i]->GetName().c_str())) {// Элемент панели иерархии
						ImGui::PushID(GameObjects[i]->GetId());
						ImGui::PopID();
						ImGui::TreePop();

					}

					if (ImGui::IsItemClicked()) {
						spdlog::info("Object with number {:08d} is selected", i);
						//: При выборе объекта, получаем его Id для изменения его параметров
						SelectedItem = GameObjects[i]->GetId();
					}
				}


				//: Добавить новый объект
				if (ImGui::Button("+ Add Object")) { 

					//: Создание объекта по умолчанию
					Lava::GameObject* newObject = new Lava::GameObject;
					newObject->GetMesh()->CreateMesh("assets/cube.obj");
					newObject->SetId((int)GameObjects.size());
					std::string nameString = "GameObject";
					newObject->SetName(nameString);
					newObject->GetMesh()->AddBaseColorTexture("");
					newObject->Transform.SetStartTranslation(glm::vec3(0.0f, 0.0f, 0.0f));
					newObject->Transform.Scale(glm::vec3(1.f, 1.f, 1.f));
					newObject->Transform.SetStartRotation(glm::vec3(0.f, 0.f, 0.f));
					newObject->GetRigidBody()->CreateRigidBody(
						newObject->GetMesh(),
						Lava::LAVA_RIGIDBODY_SHAPE_TYPE_MESH,
						1.1f,
						0.2f,
						1.9f,
						Lava::gDynamicsWorld,
						newObject->GetId()
					);
					newObject->ApplyEntityTransformToRigidbodyAndMesh();

				//	*GameObjects.push_back(newObject);
				}

				//: Если объект выбран, то его ID != -1
				if (SelectedItem != -1) { 
					if (GameObjects[SelectedItem]->GetType() == Lava::LAVA_ENTITY_TYPE_GAME_OBJECT) {
						bool opened = true;

						ImGui::Begin("Properties", &opened, ImGuiWindowFlags_NoMove);
						
						//: Изменение размеров вьюпорта в зависимости от размеров панели свойств
						Lava::gEditor3DViewport.width -= Lava::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x;
						ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x, MenubarHeight));
						ImGui::SetWindowSize(ImVec2(Lava::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x,
							Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight));

						//: Позиция в мире
						glm::vec3 objectPos = GameObjects[SelectedItem]->Transform.GetPosition();
						ImGui::SliderFloat3("Entity Position", (float*)&objectPos, -100.f, 100.f);
						GameObjects[SelectedItem]->Transform.SetStartTranslation(objectPos);
						GameObjects[SelectedItem]->ApplyEntityTransformToRigidbodyAndMesh();

						ImGui::End();

						//: Если окно свойств закрылось, то никакой объект не выбран 
						if (!opened) { 
							SelectedItem = -1;
						}
					}
				}

				ImGui::End();
			}
		}
		else {
			sceneIsPlaying = true;
		}
	}
	
}sceneEditor;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//: Главная оконная процедура
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	//: Обработка оконных забытий в ImGui
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return true;

	switch (msg) {
	case WM_KEYDOWN:
		Lava::keyPressedEventHandler.ProcessKeyDownEvent(wparam);
		break;

	case WM_KEYUP:
		Lava::keyPressedEventHandler.ProcessKeyUpEvent(wparam);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;

	case WM_MOUSEMOVE:
		Lava::mouseMoveEventHandler.SetCursorPos(LOWORD(lparam),HIWORD(lparam));
		break;


	default:
		return ::DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	}
}


//: Приложение Win32
class Application {

	HWND		hwnd;
	HINSTANCE   hInstance;
	WNDCLASS	wc;

	//: Данные для отрисовки интерфейса
	ImDrawData* ImguiDrawData;

	uint32_t	WindowWidth, 
				WindowHeight;

	double		Time, 
				DeltaTime, //: Время отрисовки одного кадра 
				LastFrameTime;
	bool		SceneIsPlaying = false;

public:
	void Init(){
		
		WindowWidth = 1366;
		WindowHeight = 768;

		Lava::gHeight = WindowHeight;
		Lava::gWidth  = WindowWidth;

		spdlog::info("WIDTH {:08d}",Lava::gWidth);

		//: Инициализация оконного класса
		wc = {0};
		wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.lpszClassName = LPCWSTR("MyWindowClass");
		wc.lpszMenuName = LPCWSTR("");
		wc.lpfnWndProc = WndProc;
		wc.hInstance = hInstance;
		wc.style = CS_OWNDC;
		::RegisterClass(&wc);
		hwnd = CreateWindow(
			wc.lpszClassName, 
			LPCWSTR("Engine"), 
			WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_POPUP, 
			0, 
			0, 
			Lava::gWidth, 
			Lava::gHeight, 
			0, 
			0,
			hInstance, 
			0);

		::ShowWindow(hwnd, SW_SHOWDEFAULT);
		::UpdateWindow(hwnd);
		
		//: Инициализация компонентов Vulkan
		Lava::initVulkan(hwnd,hInstance);
		Lava::gScene.InitScene();

		//: Инициализация ImGui
		if (LAVA_GLOBAL_ENABLE_IMGUI) {
			sceneEditor.InitEditor(hwnd);
		}
		
		Time = 0;
		LastFrameTime = 0;
		DeltaTime = 0;
		SceneIsPlaying = false;
	}

	void Execute() {
		MSG msg = { };
		while (msg.message != WM_QUIT) {
			//: Обработка событий
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}

			//: Вычисление времени выполнения одного итерации (кадра)
			Time = GetTickCount();
			DeltaTime = Time - LastFrameTime;
			DeltaTime /= 100;
			LastFrameTime = Time;

			//spdlog::info("DeltaTime ", DeltaTime);

			//: Выполнение скрипта камеры
			Lava::debugCamera.Update(floor(DeltaTime * 1000) / 1000);
			

			//: Симуляция физики твердых тел
			if (SceneIsPlaying) {
				for (size_t i = 0; i < Lava::gScene.GetVectorOfEntities().size(); i++) {
					//: Применение результата симуляции
					Lava::gScene.GetVectorOfEntities()[i]->ApplyPhysicsToEntity();
				}
				Lava::gDynamicsWorld->stepSimulation(floor(DeltaTime*1000)/1000);
			}

			if (LAVA_GLOBAL_ENABLE_IMGUI) {
				//: Формирование кадра для отрисовки интерфейса
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
				
				sceneEditor.PrepareToRender(hwnd, Lava::gScene.GetVectorOfEntities(), SceneIsPlaying);

				ImGui::Render();
				ImguiDrawData = ImGui::GetDrawData();
				ImGui::EndFrame();
			}

			//: Отрисовка объектов сцены и интерфейса
			Lava::DrawScene(
				ImguiDrawData,
				Lava::gScene
			);
		}
	}

	//: Освободить память
	void Clear() {
		//: Ожидание бездействия GPU
		vkDeviceWaitIdle(Lava::gDevice.Get());

		Lava::gScene.CleanScene();
		ImGui::DestroyContext();
		ImGui_ImplWin32_Shutdown();
		ImGui_ImplVulkan_Shutdown();
		DestroyWindow(hwnd);
		Lava::clear();
	}

	uint32_t GetWidth() {
		return WindowWidth;
	}

	uint32_t GetHeight() {
		return WindowHeight;
	}
}Application;
