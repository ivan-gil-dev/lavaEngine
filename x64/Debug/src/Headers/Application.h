#pragma once

#include "MainVulkan.h"
#include <iostream>

// �������� �����
class SceneEditor{
	bool  DemoWindowActive = true,
		  StartButtonActive,      
		  ResetPhysics = true,
		  CloseWindowMenuItem = false ;

	int   SelectedItem  = -1 ; //: ��������� ������ �� ���� ��������
	float MenubarHeight = 0 ;
public:

	//: ������������� �������� ImGui
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
		
		//: �������� ����������� Vulkan � ImGui
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

		//: �������� ������� � GPU
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

	//: ������ ������ ��� ��������� ��������� ����������
	void PrepareToRender(HWND hwnd, const std::vector<Lava::Entity*>& GameObjects, bool& sceneIsPlaying) {
		if (LAVA_GLOBAL_ENABLE_IMGUI) {

			//: Show demo window
			ImGui::ShowDemoWindow(&DemoWindowActive); 

			//: ��������� �������
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 20));
			
			//: ������ ������ ������ ��� ��������
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
					
					//: ������������ �������� �� ���� ����������� �����������
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
							//: ����� ���������� �������, ������������� ��������
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
						ResetPhysics = false; //: �������������� ���������� ������ ����� ����
					}
				}
				ImGui::EndMenu();
			}


			if (ImGui::BeginMenu(u8"View")) {
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();

			if (!sceneIsPlaying) {

				//: ������ ��������
				ImGui::Begin(u8"Hierarchy Panel", NULL, ImGuiWindowFlags_NoMove);

				//: ��������������� �������� � ����������� �� ������� 
				//  ������ ��������
				Lava::gEditor3DViewport.x = ImGui::GetWindowWidth();
				Lava::gEditor3DViewport.y = MenubarHeight;
 				Lava::gEditor3DViewport.width = Lava::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowWidth();
				Lava::gEditor3DViewport.height = Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight;
				Lava::gEditor3DScissors.extent = Lava::gSwapchain.GetInfo().imageExtent;

				//: ������������ ��������� ������ ��������
				ImGui::SetWindowPos(ImVec2(0, MenubarHeight)); 
				ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(),
					Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight)); // ��������� �� ������ ����
				
				//: ����� ������ ��������
				for (size_t i = 0; i < GameObjects.size(); i++) {
					if (ImGui::TreeNode(GameObjects[i]->GetName().c_str())) {// ������� ������ ��������
						ImGui::PushID(GameObjects[i]->GetId());
						ImGui::PopID();
						ImGui::TreePop();

					}

					if (ImGui::IsItemClicked()) {
						spdlog::info("Object with number {:08d} is selected", i);
						//: ��� ������ �������, �������� ��� Id ��� ��������� ��� ����������
						SelectedItem = GameObjects[i]->GetId();
					}
				}


				//: �������� ����� ������
				if (ImGui::Button("+ Add Object")) { 

					//: �������� ������� �� ���������
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

				//: ���� ������ ������, �� ��� ID != -1
				if (SelectedItem != -1) { 
					if (GameObjects[SelectedItem]->GetType() == Lava::LAVA_ENTITY_TYPE_GAME_OBJECT) {
						bool opened = true;

						ImGui::Begin("Properties", &opened, ImGuiWindowFlags_NoMove);
						
						//: ��������� �������� �������� � ����������� �� �������� ������ �������
						Lava::gEditor3DViewport.width -= Lava::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x;
						ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x, MenubarHeight));
						ImGui::SetWindowSize(ImVec2(Lava::gSwapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x,
							Lava::gSwapchain.GetInfo().imageExtent.height - MenubarHeight));

						//: ������� � ����
						glm::vec3 objectPos = GameObjects[SelectedItem]->Transform.GetPosition();
						ImGui::SliderFloat3("Entity Position", (float*)&objectPos, -100.f, 100.f);
						GameObjects[SelectedItem]->Transform.SetStartTranslation(objectPos);
						GameObjects[SelectedItem]->ApplyEntityTransformToRigidbodyAndMesh();

						ImGui::End();

						//: ���� ���� ������� ���������, �� ������� ������ �� ������ 
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

//: ������� ������� ���������
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	//: ��������� ������� ������� � ImGui
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


//: ���������� Win32
class Application {

	HWND		hwnd;
	HINSTANCE   hInstance;
	WNDCLASS	wc;

	//: ������ ��� ��������� ����������
	ImDrawData* ImguiDrawData;

	uint32_t	WindowWidth, 
				WindowHeight;

	double		Time, 
				DeltaTime, //: ����� ��������� ������ ����� 
				LastFrameTime;
	bool		SceneIsPlaying = false;

public:
	void Init(){
		
		WindowWidth = 1366;
		WindowHeight = 768;

		Lava::gHeight = WindowHeight;
		Lava::gWidth  = WindowWidth;

		spdlog::info("WIDTH {:08d}",Lava::gWidth);

		//: ������������� �������� ������
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
		
		//: ������������� ����������� Vulkan
		Lava::initVulkan(hwnd,hInstance);
		Lava::gScene.InitScene();

		//: ������������� ImGui
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
			//: ��������� �������
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}

			//: ���������� ������� ���������� ������ �������� (�����)
			Time = GetTickCount();
			DeltaTime = Time - LastFrameTime;
			DeltaTime /= 100;
			LastFrameTime = Time;

			//spdlog::info("DeltaTime ", DeltaTime);

			//: ���������� ������� ������
			Lava::debugCamera.Update(floor(DeltaTime * 1000) / 1000);
			

			//: ��������� ������ ������� ���
			if (SceneIsPlaying) {
				for (size_t i = 0; i < Lava::gScene.GetVectorOfEntities().size(); i++) {
					//: ���������� ���������� ���������
					Lava::gScene.GetVectorOfEntities()[i]->ApplyPhysicsToEntity();
				}
				Lava::gDynamicsWorld->stepSimulation(floor(DeltaTime*1000)/1000);
			}

			if (LAVA_GLOBAL_ENABLE_IMGUI) {
				//: ������������ ����� ��� ��������� ����������
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
				
				sceneEditor.PrepareToRender(hwnd, Lava::gScene.GetVectorOfEntities(), SceneIsPlaying);

				ImGui::Render();
				ImguiDrawData = ImGui::GetDrawData();
				ImGui::EndFrame();
			}

			//: ��������� �������� ����� � ����������
			Lava::DrawScene(
				ImguiDrawData,
				Lava::gScene
			);
		}
	}

	//: ���������� ������
	void Clear() {
		//: �������� ����������� GPU
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
