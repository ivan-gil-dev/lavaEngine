//TODO: ���������� ������ �������
#pragma once

#include "MainVulkan.h"
#include <iostream>

// �������� �����
class SceneEditor{
public:
	// ��������� ������� � �� ���������
	// � ��������� (�������/�������)
	bool  DemoWindowActive = true,
		  ShowHierarchyPanel = true,
		  ShowPropertiesPanel = true,
		  StartButtonActive,      
		  ResetPhysics = true,
		  CloseWindowMenuItem = false;
public:
	int   SelectedItem  = 1 ; //��������� ������ �� ���� ��������
	float MenubarHeight = 0 ;
public:

	// ������������� �������� ImGui
	void InitEditor(HWND hwnd){
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsClassic();
		ImGuiStyle& style = ImGui::GetStyle();
		// ���������� ��������� �� �������
		{
			style.WindowRounding = 0.0f;
			style.WindowPadding = ImVec2(2, 2);
			style.FramePadding = ImVec2(2, 2);
			style.WindowTitleAlign = ImVec2(0.5, 0.5);
			style.WindowMenuButtonPosition = ImGuiDir_Right;
		}
		
		// ������������� ImGui ��� Win32
		if (ImGui_ImplWin32_Init(hwnd) != true) {
			throw std::runtime_error("Failed to init imguiWin32 for vulkan");
		}
		
		// ��������� � ������������ Vulkan ��� ImGui
		ImGui_ImplVulkan_InitInfo init_info = {};

		// ���������� ��������� ���������� ���������
		{
			init_info.Instance = Lava::gInstance.get();
			init_info.PhysicalDevice = Lava::gPhysicalDevice.Get();
			init_info.Device = *Lava::gDevice.PGet();
			init_info.QueueFamily = Lava::gPhysicalDevice.GetQueueIndices().graphicsQueueIndex;
			init_info.Queue = Lava::gDevice.GetGraphicsQueue();
			init_info.PipelineCache = nullptr;
			init_info.DescriptorPool = Lava::gDescriptorPoolForImgui.Get();
			init_info.Allocator = nullptr;
			init_info.MinImageCount = Lava::gSwapchain.GetInfo().minImageCount;
			init_info.ImageCount = Lava::gSwapchain.GetInfo().minImageCount;
			init_info.CheckVkResultFn = imguiErrFunction;
		}
		

		ImGui_ImplVulkan_Init(&init_info, Lava::gRenderPass.GetRenderPass());

		Lava::CommandBuffer oneTimeSubmitCommandBuffer;
		// ������ ������ ������
		{
			oneTimeSubmitCommandBuffer.AllocateCommandBuffer(Lava::gDevice.Get(),Lava::gCommandPool.Get());
			oneTimeSubmitCommandBuffer.BeginCommandBuffer();
		}
		
		// ��������� ������ � GPU ��� ImGui
		ImGui_ImplVulkan_CreateFontsTexture(
			oneTimeSubmitCommandBuffer.Get()
		);

		// ��������� ������ ������
		{
			oneTimeSubmitCommandBuffer.EndCommandBuffer();
			oneTimeSubmitCommandBuffer.SubmitCommandBuffer(Lava::gDevice.GetGraphicsQueue());
			oneTimeSubmitCommandBuffer.FreeCommandBuffer(Lava::gDevice.Get(),
				Lava::gCommandPool.Get());
		}
		
		// �������� ������ �� ��������� ��������
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	// ������ ������ ��� ��������� ��������� ����������
	void PrepareToRender(HWND hwnd, const std::vector<Lava::Entity*>& Entities) {
		if (ENABLE_IMGUI) {
			// Show demo window
			ImGui::ShowDemoWindow(&DemoWindowActive); 
			// �������
			{
				ImGui::BeginMainMenuBar();
				MenubarHeight = ImGui::GetWindowHeight();
				// ���� "File"
				{
					if (ImGui::BeginMenu(u8"File")) {
						ImGui::MenuItem(u8"Exit", "ALT + F4", &CloseWindowMenuItem);
						ImGui::EndMenu();
						if (CloseWindowMenuItem) {
							SendMessage(hwnd, WM_CLOSE, 0, 0);
						}
					}
				}
				
				// ���� ��������� (���������/������ ���������)
				{
					if (ImGui::BeginMenu(u8"Simulation")) {
						ImGui::MenuItem(u8"Start/Stop", "", &StartButtonActive);

						if (StartButtonActive) {
							Lava::gIsScenePlaying = true;
							ResetPhysics = true;

							//��������������� ���� �� ����� �� ���� ������ ����������� �����������
							{
								Lava::gEditor3DView.x = 0;
								Lava::gEditor3DView.y = 0;
								Lava::gEditor3DView.height = Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight;
								Lava::gEditor3DView.width  = Lava::gSwapchain.GetInfo().imageExtent.width;
								Lava::gEditor3DScissors.extent = Lava::gSwapchain.GetInfo().imageExtent;
							}
						}
						else {
							Lava::gIsScenePlaying = false;
							// ��������� ���������
							if (ResetPhysics) {
								for (size_t i = 0; i < Entities.size(); i++) {
									// ����� ���������� �������, �������������� ��������
									{
										if (Entities[i]->GetEntityType() == Lava::LAVA_ENTITY_TYPE_GAME_OBJECT) {

											Entities[i]->Transform.ResetTransform();
											Entities[i]->ApplyEntityTransformToRigidbodyAndMesh();

											if (Entities[i]->GetRigidbody()->GetBulletRigidBody() != nullptr) {

												Entities[i]->GetRigidbody()->GetBulletRigidBody()->clearForces();
												Entities[i]->GetRigidbody()->GetBulletRigidBody()->clearGravity();
												Entities[i]->GetRigidbody()->GetBulletRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
												Entities[i]->GetRigidbody()->GetBulletRigidBody()->setAngularVelocity(btVector3(0, 0, 0));

											}
										}
									}
								}
								Lava::gDynamicsWorld->clearForces();
								Lava::gSolver->reset();
								ResetPhysics = false;
							}
						}
						ImGui::EndMenu();
					}
				}
				

				// ���� "View" (���������� ��������)
				{
					if (ImGui::BeginMenu(u8"View")) {
						ImGui::MenuItem(u8"Show Hierarchy Panel", "", &ShowHierarchyPanel);
						ImGui::MenuItem(u8"Show Properties Panel", "", &ShowPropertiesPanel);
						ImGui::MenuItem(u8"Show Skybox", "", &Lava::gShowSkybox);
						ImGui::MenuItem(u8"Show Meshes", "", &Lava::gShowMeshes);
						ImGui::MenuItem(u8"Show Rigidbody Meshes", "", &Lava::gShowRigidbodyMeshes);
						ImGui::EndMenu();
					}
				}
				
				ImGui::EndMainMenuBar();
			}
			
			// ��������� �������� 3D ����
			// � ������������ � ���������
			{
				Lava::gEditor3DView.y = MenubarHeight;
				Lava::gEditor3DView.height = Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight;
			}


			if (ShowHierarchyPanel) {
				// ������ ��������
				bool opened = true;
				ImGui::Begin(u8"Hierarchy Panel",&opened, ImGuiWindowFlags_NoMove);

				//  ��������������� ���� �� ����� � ����������� �� ������� 
				//  ������ ��������
				{
					Lava::gEditor3DView.x = ImGui::GetWindowWidth();
					Lava::gEditor3DView.width = Lava::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowWidth();

					//  ������������ ������ ������ ��������
					ImGui::SetWindowPos(ImVec2(0, MenubarHeight));
					ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(),
						Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight)); // ��������� �� ������ ����
				}
				
				//  ����� ������ ��������
				{
					for (size_t i = 0; i < Entities.size(); i++) {
						if (ImGui::TreeNode(Entities[i]->GetName().c_str())) {// ������� ������ ��������
							ImGui::PushID(Entities[i]->GetID());
							ImGui::PopID();
							ImGui::TreePop();

						}

						if (ImGui::IsItemClicked()) {
							spdlog::info("Object with number {:08d} is selected", i);

							// ��� ������ �������, �������� ��� Id ��� ��������� ��� ����������
							SelectedItem = Entities[i]->GetID();

							ShowPropertiesPanel = true;
						}
					}
				}
				

				// ������ "Add Object"
				if (ImGui::Button("+ Add Object")) { 

					// �������� ������ �������
					{
						Lava::GameObject* newObject = new Lava::GameObject;
						newObject->GetMesh()->CreateMesh("assets/cube.obj");
						newObject->SetID((int)Entities.size());
						std::string nameString = "GameObject";
						newObject->SetName(nameString);
						newObject->GetMesh()->SetBaseColorTexture("");
						newObject->Transform.Translate(glm::vec3(0.0f, 0.0f, 0.0f));
						newObject->Transform.Scale(glm::vec3(1.f, 1.f, 1.f));
						newObject->Transform.Rotate(glm::vec3(0.f, 0.f, 0.f));
						newObject->GetRigidbody()->CreateRigidBody(
							Lava::LAVA_RIGIDBODY_SHAPE_TYPE_CUBE,
							1.1f,
							0.2f,
							1.9f,
							Lava::gDynamicsWorld,
							newObject->GetID()
						);
						newObject->ApplyEntityTransformToRigidbodyAndMesh();
						//	*GameObjects.push_back(newObject);
					}
					
				}

				if (!opened){
					ShowHierarchyPanel = false;
				}

				ImGui::End();
			}
			else {
				Lava::gEditor3DView.x = 0;
				Lava::gEditor3DView.width = Lava::gSwapchain.GetInfo().imageExtent.width;
			}

			
			if (ShowPropertiesPanel) {
				if (Entities[SelectedItem]->GetEntityType() == Lava::LAVA_ENTITY_TYPE_GAME_OBJECT) {
					bool opened = true;

					ImGui::Begin("Properties", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

					// ��������� �������� 3D ���� � ����������� �� �������� ������ �������
					{
						Lava::gEditor3DView.width -= Lava::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x;

						// ��������� ��������� ������ � ����
						ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x, MenubarHeight));
						// ������ ������ ������
						ImGui::SetWindowSize(
							// ������ ���� ����� ���������� ������ 
							// �� ������ ����
							ImVec2(Lava::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x,
							// ������ ���� ����� ������ ��������
							Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight)
						);
					}
					
					// ��������
					{
						glm::vec3 objectPos = Entities[SelectedItem]->Transform.gPosition();
						glm::vec3 objectRotation = Entities[SelectedItem]->Transform.gEulerAngles();
						glm::vec3 objectRigidbodyScale = Entities[SelectedItem]->GetRigidbody()->GetDebugMesh()->Transform.gScaleValue();
						ImGui::SliderFloat3("Entity Position", (float*)&objectPos, -100.f, 100.f);
						ImGui::SliderFloat3("Entity Rotation", (float*)&objectRotation, 0.f, 360.0f);
						ImGui::SliderFloat3("Rigidbody Scale", (float*)&objectRigidbodyScale, 0.f, 300.0f);
						if (!Lava::gIsScenePlaying) {
							Entities[SelectedItem]->Transform.Translate(objectPos);
							Entities[SelectedItem]->Transform.Rotate(objectRotation);
							Entities[SelectedItem]->GetRigidbody()->SetRigidbodyScale(objectRigidbodyScale);
							Entities[SelectedItem]->ApplyEntityTransformToRigidbodyAndMesh();
						}
					}
					
					ImGui::End();

					// ���� ���� ������� ���������, �� ������� ������ �� ������ 
					if (!opened) {
						ShowPropertiesPanel = false;
					}
				}
			}

		}
		else {
		Lava::gIsScenePlaying = true;
		}
	}
	
}sceneEditor;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ������� ������� ���������
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	// ��������� ������� ������� � ImGui
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return true;

	//��������� ��������� 
	{
		switch (msg) {
		case WM_SYSKEYDOWN:
			Lava::keyPressedEventHandler.ProcessKeyDownEvent(wparam, lparam);
			break;

		case WM_SYSKEYUP:
			Lava::keyPressedEventHandler.ProcessKeyUpEvent(wparam, lparam);
			break;

		case WM_KEYDOWN:
			Lava::keyPressedEventHandler.ProcessKeyDownEvent(wparam, lparam);
			break;

		case WM_KEYUP:
			Lava::keyPressedEventHandler.ProcessKeyUpEvent(wparam, lparam);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			break;

		case WM_MOUSEMOVE:
			Lava::mouseMoveEventHandler.SetCursorPos(LOWORD(lparam), HIWORD(lparam));
			break;

		default:
			return ::DefWindowProc(hwnd, msg, wparam, lparam);
			break;
		}
	}
	
}


//: ���������� Win32
class Application {
	// ������� Win32
public:
	HWND		hwnd;
	HINSTANCE   hInstance;
	WNDCLASS	wc;

public:
	// ������ ��� ��������� ����������
	ImDrawData* ImguiDrawData;

public:
	// ������ ����
	uint32_t	WindowWidth, 
				WindowHeight;
public:
	// ���������� ��� ���������� FPS
	double		Time, 
				// ����� ��������� ������ ����� 
				DeltaTime, 
				LastFrameTime;
public:
	void Init(){
		WindowWidth = 1366;
		WindowHeight = 768;

		Lava::gHeight = WindowHeight;
		Lava::gWidth  = WindowWidth;

		spdlog::info("WIDTH {:08d}",Lava::gWidth);

		// ������������� �������� ������
		wc = {0};

		// ���������� �������� ������
		{
			wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			wc.lpszClassName = LPCWSTR("MyWindowClass");
			wc.lpszMenuName = LPCWSTR("");
			wc.lpfnWndProc = WndProc;
			wc.hInstance = hInstance;
			wc.style = CS_OWNDC;
		}
		::RegisterClass(&wc);

		// ������ ����� (��/���)
		if (Lava::gToggleFullscreen){
			hwnd = CreateWindow(
				wc.lpszClassName,
				LPCWSTR("Engine"),
				WS_POPUP | WS_VISIBLE,
				0,
				0,
				Lava::gWidth,
				Lava::gHeight,
				0,
				0,
				hInstance,
				0);

			int w = GetSystemMetrics(SM_CXSCREEN);
			int h = GetSystemMetrics(SM_CYSCREEN);
			SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, 0);

		}else{
			hwnd = CreateWindow(
				wc.lpszClassName,
				LPCWSTR("Engine"),
				WS_VISIBLE | WS_OVERLAPPED,
				0,
				0,
				Lava::gWidth,
				Lava::gHeight,
				0,
				0,
				hInstance,
				0);
		
		}
		

		::ShowWindow(hwnd, SW_SHOWDEFAULT);
		::UpdateWindow(hwnd);

		// ������������� ����������� Vulkan
		Lava::initVulkan(hwnd,hInstance);
		// �������� �������� �����
		Lava::gScene.InitScene();

		// ������������� ImGui
		if (ENABLE_IMGUI) {
			sceneEditor.InitEditor(hwnd);
		}
		else {
			Lava::gIsScenePlaying = true;
		}
		
		Time = 0;
		LastFrameTime = 0;
		DeltaTime = 0;
	}

	void Execute() {
		MSG msg = { };
		while (msg.message != WM_QUIT) {
			//��������� �������
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}

			//���������� ������� ���������� ������ �������� (�����)
			{
				Time = GetTickCount();
				DeltaTime = Time - LastFrameTime;
				DeltaTime /= 100;
				LastFrameTime = Time;
			}
			

			//spdlog::info("DeltaTime ", DeltaTime);

			// ���������� ������� ������
			Lava::debugCamera.Update(floor(DeltaTime * 1000) / 1000);
			
			//: ��������� ������ ������� ���
			if (Lava::gIsScenePlaying) {
				for (size_t i = 0; i < Lava::gScene.gVectorOfEntities().size(); i++) {
					// ���������� ���������� ��������� � ��������
					Lava::gScene.gVectorOfEntities()[i]->ApplyPhysicsToEntity();
				}
				Lava::gDynamicsWorld->stepSimulation(floor(DeltaTime*1000)/1000);
			}

			if (ENABLE_IMGUI) {
				// ������������ ����� ��� ��������� ����������
				{
					ImGui_ImplVulkan_NewFrame();
					ImGui_ImplWin32_NewFrame();
					ImGui::NewFrame();

					sceneEditor.PrepareToRender(hwnd, Lava::gScene.gVectorOfEntities());

					ImGui::Render();
					ImguiDrawData = ImGui::GetDrawData();
					ImGui::EndFrame();
				}
				
			}

			// ��������� �������� ����� � ����������
			Lava::DrawScene(
				ImguiDrawData,
				Lava::gScene
			);
		}
	}

	// ���������� ������
	void Clear() {
		// �������� ����������� GPU
		vkDeviceWaitIdle(Lava::gDevice.Get());
		Lava::gScene.CleanScene();

		if (ENABLE_IMGUI){
			ImGui::DestroyContext();
			ImGui_ImplWin32_Shutdown();
			ImGui_ImplVulkan_Shutdown();
		}

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
