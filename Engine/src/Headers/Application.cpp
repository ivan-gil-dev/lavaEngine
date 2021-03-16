#include "Application.h"

// Инициализация ImGui

void SceneEditor::InitEditor(HWND hwnd) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsClassic();
	ImGuiStyle& style = ImGui::GetStyle();
	
	{
		/* Стиль редактора
		*/
		style.WindowRounding = 0.0f;
		style.WindowPadding = ImVec2(2, 2);
		style.FramePadding = ImVec2(2, 2);
		style.WindowTitleAlign = ImVec2(0.5, 0.5);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
	}

	/* Инициализация ImGui для Win32
	*/
	if (ImGui_ImplWin32_Init(hwnd) != true) {
		throw std::runtime_error("Failed to init imguiWin32 for vulkan");
	}

	/* Структура с хэндлами компонентов Vulkan для ImGui
	*/
	ImGui_ImplVulkan_InitInfo init_info = {};

	
	{
		init_info.Instance = Engine::Globals::gInstance.get();
		init_info.PhysicalDevice = Engine::Globals::gPhysicalDevice.Get();
		init_info.Device = *Engine::Globals::gDevice.PGet();
		init_info.QueueFamily = Engine::Globals::gPhysicalDevice.GetQueueIndices().graphicsQueueIndex;
		init_info.Queue = Engine::Globals::gDevice.GetGraphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = Engine::Globals::gDescriptorPoolForImgui.Get();
		init_info.Allocator = nullptr;
		init_info.MinImageCount = Engine::Globals::gSwapchain.GetInfo().minImageCount;
		init_info.ImageCount = Engine::Globals::gSwapchain.GetInfo().minImageCount;
		init_info.CheckVkResultFn = imguiErrFunction;
	}


	ImGui_ImplVulkan_Init(&init_info, Engine::Globals::gRenderPass.GetRenderPass());

	Engine::CommandBuffer oneTimeSubmitCommandBuffer;

	/* Начать запись команд
	*/
	{
		oneTimeSubmitCommandBuffer.AllocateCommandBuffer(Engine::Globals::gDevice.Get(),
			Engine::Globals::gCommandPool.Get());
		oneTimeSubmitCommandBuffer.BeginCommandBuffer();
	}

	/* Загрузить шрифты для ImGui
	*/
	ImGui_ImplVulkan_CreateFontsTexture(
		oneTimeSubmitCommandBuffer.Get()
	);

	/* Закончить запись команд
	*/
	{
		oneTimeSubmitCommandBuffer.EndCommandBuffer();
		oneTimeSubmitCommandBuffer.SubmitCommandBuffer(Engine::Globals::gDevice.GetGraphicsQueue());
		oneTimeSubmitCommandBuffer.FreeCommandBuffer(Engine::Globals::gDevice.Get(),
			Engine::Globals::gCommandPool.Get());
	}

	/* Очистить память от временных объектов
	*/
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

// Вывод редактора
void SceneEditor::DrawEditor(HWND hwnd, const std::vector<Engine::Entity*>& Entities) {
	if (ENABLE_IMGUI) {
		
		// Демо ImGui
		ImGui::ShowDemoWindow(&DemoWindowActive);

		// Обработка менюбара
		{
			ImGui::BeginMainMenuBar();
			MenubarHeight = ImGui::GetWindowHeight();
			
			// Меню "Файл"
			{
				if (ImGui::BeginMenu(u8"File")) {
					ImGui::MenuItem(u8"Exit", "ALT + F4", &CloseWindow);
					ImGui::EndMenu();
					if (CloseWindow) {
						SendMessage(hwnd, WM_CLOSE, 0, 0);
					}
				}
			}

			// Меню симуляции (остановка/начало симуляции)
			{
				if (ImGui::BeginMenu(u8"Simulation")) {
					ImGui::MenuItem(u8"Start", "", &StartButtonActive);

					if (StartButtonActive) {
						Engine::Globals::gIsScenePlaying = true;
						ResetPhysics = true;

						{
							Engine::Globals::gEditor3DView.x = 0;
							Engine::Globals::gEditor3DView.y = 0;
							Engine::Globals::gEditor3DView.height = static_cast<float>(Engine::Globals::gSwapchain.GetInfo().imageExtent.height - MenubarHeight);
							Engine::Globals::gEditor3DView.width  = static_cast<float>(Engine::Globals::gSwapchain.GetInfo().imageExtent.width);
							Engine::Globals::gEditor3DScissors.extent = Engine::Globals::gSwapchain.GetInfo().imageExtent;
						}
					}
					else {
						Engine::Globals::gIsScenePlaying = false;
						
						//_Сброс физических параметров объектов
						if (ResetPhysics) {
							for (size_t i = 0; i < Entities.size(); i++) {
								Entities[i]->Transform.ResetTransform();
								if (Entities[i]->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT) {
									if (((Engine::GameObject*)Entities[i])->pRigidBody != nullptr) {

										((Engine::GameObject*)Entities[i])->ApplyEntityTransformToRigidbody();
										btRigidBody *pRigidbody = ((Engine::GameObject*)Entities[i])->pRigidBody->GetBulletRigidBody();
										if (pRigidbody->getMass() != 0) {
											pRigidbody->clearForces();
											pRigidbody->clearGravity();
											pRigidbody->setLinearVelocity(btVector3(0, 0, 0));
											pRigidbody->setAngularVelocity(btVector3(0, 0, 0));
										}
									}
								}
							}
							Engine::Globals::gDynamicsWorld->clearForces();
							Engine::Globals::gSolver->reset();
							ResetPhysics = false;
						}
					}
					ImGui::EndMenu();
				}
			}

			// Меню "View" (Управление панелями)
			{
				if (ImGui::BeginMenu(u8"View")) {
					ImGui::MenuItem(u8"Hierarchy Panel", "", &ShowHierarchyPanel);
					ImGui::MenuItem(u8"Properties Panel", "", &ShowPropertiesPanel);
					ImGui::MenuItem(u8"Skybox", "", &Engine::Globals::gShowSkybox);
					ImGui::MenuItem(u8"Mesh", "", &Engine::Globals::gShowMeshes);
					ImGui::MenuItem(u8"Rigidbody Mesh", "", &Engine::Globals::gShowRigidbodyMeshes);
					ImGui::EndMenu();
				}
			}

			ImGui::EndMainMenuBar();
		}

		// Изменение размеров 3D вьюпорта
		// в соответствии с менюбаром
		{
			Engine::Globals::gEditor3DView.y = MenubarHeight;
			Engine::Globals::gEditor3DView.height = Engine::Globals::gSwapchain.GetInfo().imageExtent.height - MenubarHeight;
		}


		if (ShowHierarchyPanel) {

			// Панель иерархии
			bool opened = true;
			ImGui::Begin(u8"Hierarchy", &opened, ImGuiWindowFlags_NoMove);

			// Масштабирование вида на сцену в зависимости от размера 
			// панели иерархии
			{
				Engine::Globals::gEditor3DView.x = ImGui::GetWindowWidth();
				Engine::Globals::gEditor3DView.width = Engine::Globals::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowWidth();

				//_Учесть высоту менюбара при выводе панели иерархии
				ImGui::SetWindowPos(ImVec2(0, MenubarHeight));

				ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(),
					Engine::Globals::gSwapchain.GetInfo().imageExtent.height - MenubarHeight)
				); 
			}

			// Список объектов
			{
				for (size_t i = 0; i < Entities.size(); i++) {
					if (ImGui::TreeNode(Entities[i]->GetName().c_str())) {
						ImGui::PushID(Entities[i]->GetID());
						ImGui::PopID();
						ImGui::TreePop();
					}

					if (ImGui::IsItemClicked()) {
						spdlog::info("Object with number {:08d} is selected", i);

						//_Получение ID выбранного объекта
						SelectedItem_ID = Entities[i]->GetID();

						ShowPropertiesPanel = true;
					}
				}
			}


			
			if (ImGui::Button(u8"Add+")) {

				// Создание нового объекта
				{
					Engine::GameObject* newObject = new Engine::GameObject;
					newObject->pMesh->CreateMesh("assets/cube.obj");
					newObject->SetID((int)Entities.size());
					std::string nameString = "GameObject";
					newObject->SetName(nameString);
					newObject->Transform.Translate(glm::vec3(0.0f, 0.0f, 0.0f));
					newObject->Transform.Scale(glm::vec3(1.f, 1.f, 1.f));
					newObject->Transform.Rotate(glm::vec3(0.f, 0.f, 0.f));
					newObject->pRigidBody->CreateRigidBody(
						Engine::RIGIDBODY_SHAPE_TYPE_CUBE,
						1.1f,
						0.2f,
						1.9f,
						Engine::Globals::gDynamicsWorld,
						newObject->GetID()
					);
					newObject->ApplyEntityTransformToRigidbody();
					//	*GameObjects.push_back(newObject);
				}

			}

			if (!opened) {
				ShowHierarchyPanel = false;
			}

			ImGui::End();
		}
		else {
			// Если панель иерархии не выведена,
			// то растянуть вьюпорт сцены
			Engine::Globals::gEditor3DView.x = 0;
			Engine::Globals::gEditor3DView.width = static_cast<float>(Engine::Globals::gSwapchain.GetInfo().imageExtent.width);
		}


		if (ShowPropertiesPanel) {
			bool opened = true;
			switch (Entities[SelectedItem_ID]->GetEntityType()) {
			case Engine::ENTITY_TYPE_GAME_OBJECT:

				ImGui::Begin(u8"Properties", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

				// Расположение панели свойств справа
				{
					Engine::Globals::gEditor3DView.width -= Engine::Globals::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x;

		
					ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x, MenubarHeight));
					ImGui::SetWindowSize(
						ImVec2(Engine::Globals::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x,
							   Engine::Globals::gSwapchain.GetInfo().imageExtent.height - MenubarHeight)
					);
				}

				// Слайдеры
				{
					glm::vec3 objectPos = Entities[SelectedItem_ID]->Transform.GetPosition();
					glm::vec3 objectRotation = Entities[SelectedItem_ID]->Transform.GetEulerAngles();
					glm::vec3 objectRigidbodyScale;
					ImGui::DragFloat3("Position", (float*)&objectPos, 1.0f, -100.f, 100.f);
					ImGui::DragFloat3("Rotation", (float*)&objectRotation, 1.0f, 0.f, 360.0f);
					if (((Engine::GameObject*)Entities[SelectedItem_ID])->pRigidBody != nullptr) {
						objectRigidbodyScale = ((Engine::GameObject*)Entities[SelectedItem_ID])->pRigidBody->pGetDebugMesh()->Transform.GetScaleValue();
						ImGui::DragFloat3("Rigidbody Scale", (float*)&objectRigidbodyScale, 1.0f, 0.f, 300.0f);
					}
					if (!Engine::Globals::gIsScenePlaying) {
						Entities[SelectedItem_ID]->Transform.Translate(objectPos);
						Entities[SelectedItem_ID]->Transform.Rotate(objectRotation);
						if (((Engine::GameObject*)Entities[SelectedItem_ID])->pRigidBody != nullptr) {
							((Engine::GameObject*)Entities[SelectedItem_ID])->pRigidBody->SetRigidbodyScale(objectRigidbodyScale);
							((Engine::GameObject*)Entities[SelectedItem_ID])->ApplyEntityTransformToRigidbody();
						}
					}
				}

				ImGui::End();
				break;

			case Engine::ENTITY_TYPE_SPOTLIGHT_OBJECT:
				ImGui::Begin("Properties", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

				// Расположение панели свойств справа
				{
					Engine::Globals::gEditor3DView.width -= Engine::Globals::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x;

					ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x, MenubarHeight));
					ImGui::SetWindowSize(
						ImVec2(Engine::Globals::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x,
							Engine::Globals::gSwapchain.GetInfo().imageExtent.height - MenubarHeight)
					);
				}

			
				{
					glm::vec3 objectPos = Entities[SelectedItem_ID]->Transform.GetPosition();
					glm::vec3 objectRotation = Entities[SelectedItem_ID]->Transform.GetEulerAngles();
					ImGui::DragFloat3("Position", (float*)&objectPos, 1.0f, -100.f, 100.f);
					if (!Engine::Globals::gIsScenePlaying) {
						Entities[SelectedItem_ID]->Transform.Translate(objectPos);
					}
				}

				ImGui::End();
				break;

			default:
				break;
			}
			if (!opened) {
				ShowPropertiesPanel = false;
			}
		}

	}
	else {
		Engine::Globals::gIsScenePlaying = true;
	}
}

// Настройка компонентов приложения
void Application::Init() {
	WindowWidth = 1366;
	WindowHeight = 768;

	Engine::Globals::gHeight = WindowHeight;
	Engine::Globals::gWidth = WindowWidth;

	{
		const wchar_t CLASS_NAME[] = L"Engine";
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = CLASS_NAME;
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	}
	::RegisterClassEx(&wc);

	if (Engine::Globals::gToggleFullscreen) {
		hwnd = CreateWindowEx(
			0,
			wc.lpszClassName,
			LPCWSTR("Engine"),
			WS_POPUP | WS_VISIBLE,
			0,
			0,
			int(Engine::Globals::gWidth),
			int(Engine::Globals::gHeight),
			0,
			0,
			hInstance,
			0);

		int w = GetSystemMetrics(SM_CXSCREEN);
		int h = GetSystemMetrics(SM_CYSCREEN);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, 0);

	}
	else {
		hwnd = CreateWindowEx(0,
			wc.lpszClassName,
			LPCWSTR("Engine"),
			WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			0,
			0,
			Engine::Globals::gWidth,
			Engine::Globals::gHeight,
			0,
			0,
			hInstance,
			0);
	}

	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Настройка компонентов Vulkan
	Engine::initVulkan(hwnd, hInstance);

	// Демо-сцена (TODO: загрузка из JSON)
	Engine::Globals::gScene = new Engine::Scene;
	Engine::Globals::gScene->Demo();
	//Engine::Globals::gScene->SomeCode();
	// Настройка ImGui
	if (ENABLE_IMGUI) {
		sceneEditor.InitEditor(hwnd);
	}
	else {
		Engine::Globals::gIsScenePlaying = true;
	}

	Time = 0;
	LastFrameTime = 0;
	Engine::Globals::DeltaTime = 0;
}

// Главный цикл
void Application::Execute() {
	MSG msg = { };
	while (msg.message != WM_QUIT) {
		// Обработка событий
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		// Расчет времени итерации цикла
		{
			Time = GetTickCount();
			Engine::Globals::DeltaTime = Time - LastFrameTime;
			Engine::Globals::DeltaTime /= 100;
			LastFrameTime = Time;
		}

		for (size_t i = 0; i < Engine::Globals::gScene->GetVectorOfEntities().size(); i++){
			
		}

		//spdlog::info("DeltaTime ", DeltaTime);
		Engine::Globals::debugCamera.Update(floor(Engine::Globals::DeltaTime * 100) / 100);


		if (Engine::Globals::gIsScenePlaying) {
			for (size_t i = 0; i < Engine::Globals::gScene->GetVectorOfEntities().size(); i++) {
				// Применение физической модели к графической
				if (Engine::Globals::gScene->GetVectorOfEntities()[i]->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT) {
					((Engine::GameObject*)Engine::Globals::gScene->GetVectorOfEntities()[i])->ApplyPhysicsToEntity();
				}

				if (Engine::Globals::gScene->GetVectorOfEntities()[i]->GetName() == "Arena") {
					Engine::Globals::gScene->GetVectorOfEntities()[i]->Transform.Rotate(glm::vec3(0.0f,
						Engine::Globals::gScene->GetVectorOfEntities()[i]->Transform.GetEulerAngles().y+10.0f*Engine::Globals::DeltaTime,
						0.0f));
					((Engine::GameObject*)Engine::Globals::gScene->GetVectorOfEntities()[i])->ApplyEntityTransformToRigidbody();
				}
			}
			Engine::Globals::gDynamicsWorld->stepSimulation(floor(Engine::Globals::DeltaTime * 1000) / 1000);
		}

		if (ENABLE_IMGUI) {

			// Формирование данных ImGUI для передачи в GPU
			{
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				sceneEditor.DrawEditor(hwnd, Engine::Globals::gScene->GetVectorOfEntities());

				ImGui::Render();
				ImguiDrawData = ImGui::GetDrawData();
				ImGui::EndFrame();
			}

		}

		//_Вывод сцены и редактора
		Engine::DrawScene(
			ImguiDrawData,
			Engine::Globals::gScene
		);
	}
}

// Освободить ресурсы системы
void Application::Clear() {
	// Ожидание бездействия GPU
	vkDeviceWaitIdle(Engine::Globals::gDevice.Get());

	Engine::Globals::gScene->CleanScene();
	delete Engine::Globals::gScene;

	if (ENABLE_IMGUI) {
		ImGui::DestroyContext();
		ImGui_ImplWin32_Shutdown();
		ImGui_ImplVulkan_Shutdown();
	}

	DestroyWindow(hwnd);

	Engine::clear();
}

uint32_t Application::GetWidth() {
	return WindowWidth;
}

uint32_t Application::GetHeight() {
	return WindowHeight;
}
