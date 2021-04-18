#include "Application.h"

//���������� ImGui
void SceneEditor::InitEditor(HWND hwnd) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//���������� ���������
	ImGui::StyleColorsClassic();
	ImGuiStyle& style = ImGui::GetStyle();{
		style.WindowRounding = 0.0f;
		style.WindowPadding = ImVec2(2, 2);
		style.FramePadding = ImVec2(2, 2);
		style.WindowTitleAlign = ImVec2(0.5, 0.5);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
	}

	//������������ ImGui ��� WinAPI
	if (ImGui_ImplWin32_Init(hwnd) != true) {
		std::string error = "Failed to init imgui for win32";
		throw std::runtime_error(error);
	}

	//�������� ������� Vulkan � ImGui
	ImGui_ImplVulkan_InitInfo init_info = {};{
		init_info.Instance        = Engine::renderer.instance.get();
		init_info.PhysicalDevice  = Engine::renderer.physicalDevice.Get();
		init_info.Device          = *Engine::renderer.device.PGet();
		init_info.QueueFamily     = Engine::renderer.physicalDevice.GetQueueIndices().graphicsQueueIndex;
		init_info.Queue           = Engine::renderer.device.GetGraphicsQueue();
		init_info.PipelineCache   = nullptr;
		init_info.DescriptorPool  = Engine::renderer.descriptorPoolForImgui.Get();
		init_info.Allocator       = nullptr;
		init_info.MinImageCount   = Engine::renderer.swapchain.GetInfo().minImageCount;
		init_info.ImageCount      = Engine::renderer.swapchain.GetInfo().minImageCount;
		init_info.CheckVkResultFn = imguiErrFunction;
	}
	
	//������������ ImGui ��� Vulkan
	if (!ImGui_ImplVulkan_Init(&init_info, Engine::renderer.renderPass.GetRenderPass())){
		std::string error = "Failed to init imgui for Vulkan";
		throw std::runtime_error(error);
	}
	
	Engine::CommandBuffer oneTimeSubmitCommandBuffer;

	//������ ������
	oneTimeSubmitCommandBuffer.AllocateCommandBuffer(Engine::renderer.device.Get(),Engine::renderer.commandPool.Get());
	oneTimeSubmitCommandBuffer.BeginCommandBuffer();
	
		//�������� ������� � ImGui
		if (!ImGui_ImplVulkan_CreateFontsTexture(oneTimeSubmitCommandBuffer.Get())) {
			std::string error = "Failed to load fonts to Imgui";
			throw std::runtime_error(error);
		}
	
	//����� ������ ������
	oneTimeSubmitCommandBuffer.EndCommandBuffer();
	oneTimeSubmitCommandBuffer.SubmitCommandBuffer(Engine::renderer.device.GetGraphicsQueue());
	oneTimeSubmitCommandBuffer.FreeCommandBuffer(Engine::renderer.device.Get(),
		Engine::renderer.commandPool.Get());
	
	//���������� ������� ��� �������� ������
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

//���������� ����������
void SceneEditor::DrawEditor(HWND hwnd, const std::vector<Engine::Entity*>* Entities) {
	if (ENABLE_IMGUI) {
		
		ImGui::ShowDemoWindow(&DemoWindowActive);//���� ImGui
		
		ImGui::BeginMainMenuBar();//�������

			MenubarHeight = ImGui::GetWindowHeight();

			//��������� �� ������ 3D �������� ������ ��������
			Engine::renderer.rendererViewport.y = MenubarHeight;
			Engine::renderer.rendererViewport.height = Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight;

			//����: ����
			if (ImGui::BeginMenu(u8"File")) {
				ImGui::MenuItem(u8"Exit", "ALT + F4", &CloseWindow);
				ImGui::EndMenu();
				if (CloseWindow) {
					SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
			}

			//����� ����
			if (ImGui::BeginMenu(u8"Simulation")) {//����: ������ ���������
				ImGui::MenuItem(u8"Start", "", &StartButtonActive);

				if (StartButtonActive) {//��������� ������� ������ "�����"

					Engine::Globals::gIsScenePlaying = true; //������ �����
					ResetPhysics = true; //������ ����� �������� ����� ��������� ���������
						
					/*Engine::renderer.rendererViewport.x = 0;
					Engine::renderer.rendererViewport.y = 0;
					Engine::renderer.rendererViewport.height = static_cast<float>(Engine::Globals::gSwapchain.GetInfo().imageExtent.height - MenubarHeight);
					Engine::renderer.rendererViewport.width = static_cast<float>(Engine::Globals::gSwapchain.GetInfo().imageExtent.width);
					Engine::Globals::gEditor3DScissors.extent = Engine::Globals::gSwapchain.GetInfo().imageExtent;*/
						
				}
				else {
					Engine::Globals::gIsScenePlaying = false;
						
				
					if (ResetPhysics) {//����� ���������� ��������
						for (size_t i = 0; i < Entities->size(); i++) {
							Entities->at(i)->Transform.ResetTransform();
							if (Entities->at(i)->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT) {
								Engine::GameObject* obj = (Engine::GameObject*)Entities->at(i);
								if (obj->pGetComponent<Engine::RigidBody*>() != nullptr) {
									obj->ApplyEntityTransformToRigidbody();

									btRigidBody *pRigidbody = obj->pGetComponent<Engine::RigidBody*>()->GetBulletRigidBody();

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
						ResetPhysics = false;
					}
				}
				ImGui::EndMenu();//����� ����
			}
		
			
		
			if (ImGui::BeginMenu(u8"View")) { //����: "���"
				ImGui::MenuItem(u8"Hierarchy Panel", "", &ShowHierarchyPanel);
				ImGui::MenuItem(u8"Properties Panel", "", &ShowPropertiesPanel);
				ImGui::MenuItem(u8"Skybox", "", &Engine::Globals::gShowSkybox);
				ImGui::MenuItem(u8"Mesh", "", &Engine::Globals::gShowMeshes);
				ImGui::MenuItem(u8"Rigidbody Mesh", "", &Engine::Globals::gShowRigidbodyMeshes);
				ImGui::EndMenu(); //����� ����
			}
		ImGui::EndMainMenuBar(); //����� ��������
	
		//������ ��������
		if (ShowHierarchyPanel) {
			bool opened = true;
			ImGui::Begin(u8"Hierarchy", &opened, ImGuiWindowFlags_NoMove);

			//���������� 3D �������� ������ �� ������ ��������
			Engine::renderer.rendererViewport.x = ImGui::GetWindowWidth();
			Engine::renderer.rendererViewport.width = Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowWidth();

			//������ ������ �������� ��� ������ ������ ��������
			ImGui::SetWindowPos(ImVec2(0, MenubarHeight));

			ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(),
				Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
			);

			if (ImGui::GetWindowWidth()>Engine::Globals::gWidth/2){
				ImGui::SetWindowSize(ImVec2(Engine::Globals::gWidth / 2,
					Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
				);
			}
				
			//������ ��������
			for (size_t i = 0; i < Entities->size(); i++) {
				if (ImGui::TreeNode(Entities->at(i)->GetName().c_str())) {
					ImGui::PushID(Entities->at(i)->GetID());
					ImGui::PopID();
					ImGui::TreePop();
				}

				if (ImGui::IsItemClicked()) {
					spdlog::info("Object with number {:08d} is selected", i);

					//��������� ID ���������� �������
					SelectedItem_ID = Entities->at(i)->GetID();
					ShowPropertiesPanel = true;
				}
			}
			


			//TODO: ���������� �������
			//if (ImGui::Button(u8"Add+")) {

			//	//<>
			//	{
			//		Engine::GameObject* newObject = new Engine::GameObject;
			//		newObject->pMesh->CreateMesh("assets/cube.obj");
			//		newObject->SetID((int)Entities.size());
			//		std::string nameString = "GameObject";
			//		newObject->SetName(nameString);
			//		newObject->Transform.Translate(glm::vec3(0.0f, 0.0f, 0.0f));
			//		newObject->Transform.Scale(glm::vec3(1.f, 1.f, 1.f));
			//		newObject->Transform.Rotate(glm::vec3(0.f, 0.f, 0.f));
			//		newObject->pRigidBody->CreateRigidBody(
			//			Engine::RIGIDBODY_SHAPE_TYPE_CUBE,
			//			1.1f,
			//			0.2f,
			//			1.9f,
			//			Engine::Globals::gDynamicsWorld,
			//			newObject->GetID()
			//		);
			//		newObject->ApplyEntityTransformToRigidbody();
			//		//	*GameObjects.push_back(newObject);
			//	}

			//}

			if (!opened) {
				ShowHierarchyPanel = false;
			}

			ImGui::End();
		}
		else {
			//���������� 3D �������� ���� ������ �������� �� ����������
			Engine::renderer.rendererViewport.x = 0;
			Engine::renderer.rendererViewport.width = static_cast<float>(Engine::renderer.swapchain.GetInfo().imageExtent.width);
		}
		//����� ������ ��������


	
		if (ShowPropertiesPanel) {	//������ �������
			bool opened = true;

			ImGui::Begin(u8"Properties", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x, MenubarHeight));
            ImGui::SetWindowSize(
                ImVec2(Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x,
                    Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
            );

            //������������ 3D �������� ����� �� ������ �������
            Engine::renderer.rendererViewport.width -= Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x;

			if (SelectedItem_ID>=0){
                switch (Entities->at(SelectedItem_ID)->GetEntityType()) {

                case Engine::ENTITY_TYPE_GAME_OBJECT://������ ������� ��� �������

                {//����������� ������� ����������
                    glm::vec3 objectPos = Entities->at(SelectedItem_ID)->Transform.GetPosition();
                    glm::vec3 objectRotation = Entities->at(SelectedItem_ID)->Transform.GetEulerAngles();
                    glm::vec3 objectScale = Entities->at(SelectedItem_ID)->Transform.GetScaleValue();
                    glm::vec3 objectRigidbodyScale;

                    if (ImGui::CollapsingHeader(u8"Translation")) {
                        ImGui::DragFloat3("Position", (float*)&objectPos, 1.0f, -100.f, 100.f);
                        ImGui::DragFloat3("Rotation", (float*)&objectRotation, 1.0f, 0.f, 360.0f);
                        ImGui::DragFloat3("Scale", (float*)&objectScale, 1.0f, 0.f, 100.0f);
                    }

                    if (((Engine::GameObject*)Entities->at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>() != nullptr) {
                        objectRigidbodyScale = ((Engine::GameObject*)Entities->at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->pGetDebugMesh()->Transform.GetScaleValue();
                        if (ImGui::CollapsingHeader(u8"Rigidbody")) {
                            ImGui::DragFloat3("Rigidbody Scale", (float*)&objectRigidbodyScale, 0.1f, -1.f, 1.0f);
                        }

                    }

                    //���� ����� �� �������������, �� ��������� ����� ��������
                    if (!Engine::Globals::gIsScenePlaying) {
						Entities->at(SelectedItem_ID)->Transform.Translate(objectPos);
						Entities->at(SelectedItem_ID)->Transform.Rotate(objectRotation);
						Entities->at(SelectedItem_ID)->Transform.Scale(objectScale);

                        if (((Engine::GameObject*)Entities->at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>() != nullptr) {
                            ((Engine::GameObject*)Entities->at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->SetRigidbodyScale(objectRigidbodyScale);
                            ((Engine::GameObject*)Entities->at(SelectedItem_ID))->ApplyEntityTransformToRigidbody();
                        }
                    }
                }

                break;

                //������ ������� ��� ��������� ��������� �����
                case Engine::ENTITY_TYPE_POINTLIGHT_OBJECT:

                    glm::vec3 objectPos = Entities->at(SelectedItem_ID)->Transform.GetPosition();

                    if (ImGui::CollapsingHeader(u8"Translation")) {
                        ImGui::DragFloat3("Position", (float*)&objectPos, 1.0f, -100.f, 100.f);
                    }

                    //���� ����� �� �������������, �� ��������� ����� ��������
                    if (!Engine::Globals::gIsScenePlaying) {
						Entities->at(SelectedItem_ID)->Transform.Translate(objectPos);
                    }

                    break;

                case Engine::ENTITY_TYPE_DIRECTIONAL_LIGHT_OBJECT:
                    Engine::DataTypes::DirectionalLightAttributes_t dirLightAttributes = *((Engine::DirectionalLightObject*)Entities->at(SelectedItem_ID))->pGetDirectionalLightUniformData();

                    ImGui::DragFloat3("Direction", (float*)&dirLightAttributes.lightDirection, 0.1f, -1.f, 1.f, "%.3f", 0.1f);
                    ImGui::DragFloat3("Color", (float*)&dirLightAttributes.lightColor, 0.1f, 0.f, 1.f, "%.3f", 0.1f);

                    if (!Engine::Globals::gIsScenePlaying) {
                        *((Engine::DirectionalLightObject*)Entities->at(SelectedItem_ID))->pGetDirectionalLightUniformData() = dirLightAttributes;
                    }

                    break;

                default:
                    break;
                }
			}
			

			ImGui::End();
			if (!opened) {
				ShowPropertiesPanel = false;
			}
		}
		//����� ������ �������

	}
	else {
		Engine::Globals::gIsScenePlaying = true;
	}
}

//�������������� ��������� ����������
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

	//������ �����
	if (Engine::Globals::gToggleFullscreen) {
		hwnd = CreateWindowEx(
			0,
			wc.lpszClassName,
			L"Engine",
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
		hwnd = CreateWindowEx(
			WS_EX_OVERLAPPEDWINDOW,
			wc.lpszClassName,
			L"Engine",
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



	//�������������� ��������� ����������� Vulkan
	Engine::renderer.initVulkan(hwnd, hInstance);

	
	Engine::Globals::gScene = new Engine::Scene;
	
	Engine::Globals::gScene->Demo();//������������� Demo() ��� ����������
	Engine::Globals::gScene->SetScrypts();

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

//������� ����
void Application::Execute() {
	MSG msg = { };
	while (msg.message != WM_QUIT) {

		//��������� �������
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		//������ ������� �������� �����
		{
			Time = GetTickCount();
			Engine::Globals::DeltaTime = Time - LastFrameTime;
			Engine::Globals::DeltaTime /= 100;
			LastFrameTime = Time;
		}

		//spdlog::info("DeltaTime ", DeltaTime);
		
		

		if (Engine::Globals::gIsScenePlaying) {
			for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfEntities()->size(); i++) {

				//���������� ���������� ������ � �����������
				if (Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT) {
					((Engine::GameObject*)Engine::Globals::gScene->pGetVectorOfEntities()->at(i))->ApplyPhysicsToEntity();
				}

				Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->Update();
			}
			Engine::Globals::gDynamicsWorld->stepSimulation(floor(Engine::Globals::DeltaTime * 1000) / 1000);
		}

		

		if (ENABLE_IMGUI) {
			{//������������ ������ ImGUI ��� �������� � GPU
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				sceneEditor.DrawEditor(hwnd, Engine::Globals::gScene->pGetVectorOfEntities());

				ImGui::Render();
				ImguiDrawData = ImGui::GetDrawData();
				ImGui::EndFrame();
			}

		}

		//����� ����� � ��������� (��� ����� ��� ���������)
		//if (!Engine::Globals::gIsScenePlaying){
            sceneEditor.editorCamera.Update();
            Engine::renderer.DrawScene(
                ImguiDrawData,
                Engine::Globals::gScene,
                sceneEditor.editorCamera
            );
		//}
		
	}
}

//<������������ ��������>
void Application::Clear() {
	//<�������� ����������� GPU>
	vkDeviceWaitIdle(Engine::renderer.device.Get());

	Engine::Globals::gScene->CleanScene();
	delete Engine::Globals::gScene;

	if (ENABLE_IMGUI) {
		ImGui::DestroyContext();
		ImGui_ImplWin32_Shutdown();
		ImGui_ImplVulkan_Shutdown();
	}

	DestroyWindow(hwnd);

	Engine::renderer.clear();
}

uint32_t Application::GetWidth() {
	return WindowWidth;
}

uint32_t Application::GetHeight() {
	return WindowHeight;
}
