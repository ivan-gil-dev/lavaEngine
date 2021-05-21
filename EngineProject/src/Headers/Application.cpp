#include "Application.h"
#include <shobjidl.h> 
#include <algorithm>
#include "Renderer/Renderer.h"
#include <Shlwapi.h>


typedef void (*DemoFunc)(
	Engine::Scene*,
	btDynamicsWorld* 
    );
DemoFunc demo;

//Подготовка ImGui

struct InputTextCallback_UserData
{
    std::string* Str;
    ImGuiInputTextCallback  ChainCallback;
    void* ChainCallbackUserData;
};

int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        std::string* str = user_data->Str;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }
    else if (user_data->ChainCallback)
    {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

std::string WinApiOpenDialog() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
        COINIT_DISABLE_OLE1DDE);

	std::string result;

    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        std::wstring str(pszFilePath);
                        std::string path(str.begin(), str.end());

                        TCHAR buffer[MAX_PATH] = { 0 };
						GetCurrentDirectory(MAX_PATH, buffer);
						std::wstring buffer2 = buffer;
						std::string currentDir(buffer2.begin(), buffer2.end());
						//std::string out;

						char out[MAX_PATH] = "";
						char *p = new char[path.length()+1];
						strcpy(p, path.c_str());
						char *p2 = new char[currentDir.length()+1];
						strcpy(p2, currentDir.c_str());

						PathRelativePathToA(out, p2, FILE_ATTRIBUTE_DIRECTORY,
							p, FILE_ATTRIBUTE_NORMAL);


						delete []p;
						delete []p2;


						std::string path2(out);
						result = path2;
                      
                   
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
	return result;
}

std::string WinApiSaveDialog() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
        COINIT_DISABLE_OLE1DDE);
	std::string result;

    if (SUCCEEDED(hr))
    {
        IFileSaveDialog* pFileSave;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
            IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

        if (SUCCEEDED(hr))
        {
            // Show the Open dialog box.
            hr = pFileSave->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileSave->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        std::wstring str(pszFilePath);
                        std::string path(str.begin(), str.end());
						result = path;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileSave->Release();
        }
        CoUninitialize();
    }
	return result;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    //Обработка оконных событий в ImGui
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
        if ((HIWORD(lparam) & 0x4000) == 0) {
            Engine::Globals::keyPressedEventHandler.ProcessKeyDownEvent(wparam, lparam);
        }

        return true;
        break;

    case WM_CHAR:
        if (wparam == VK_RETURN) {
            return 0;
        }
        break;

    case WM_KEYUP:
        Engine::Globals::keyPressedEventHandler.ProcessKeyUpEvent(wparam, lparam);
        return true;
        break;

    case WM_SIZE:
        //std::cout << "CALL TO WM_SIZE" << wparam << std::endl;

        if (Engine::renderer.device.Get() != VK_NULL_HANDLE) {
            Engine::renderer.recreateSwapchain();
        }

        return true;
        break;

        /*	case WM_MOVING:
                    if (Engine::renderer.device.Get() != VK_NULL_HANDLE) {
                        Engine::renderer.recreateSwapchain();
                    }
                    break;*/

    case WM_SIZING:
        if (Engine::renderer.device.Get() != VK_NULL_HANDLE) {
            Engine::renderer.recreateSwapchain();
        }

        return true;

        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        break;

    case WM_MOUSEMOVE:
        Engine::Globals::cursorPosition.SetCursorPosFromWinAPI(LOWORD(lparam), HIWORD(lparam));
        return true;
        break;

    }
    return ::DefWindowProc(hwnd, msg, wparam, lparam);
}

void SceneEditor::InitEditor(HWND hwnd) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//Визуальные настройки
	ImGui::StyleColorsClassic();
	ImGuiStyle& style = ImGui::GetStyle();{
		style.WindowRounding = 0.0f;
		style.WindowPadding = ImVec2(2, 2);
		style.FramePadding = ImVec2(2, 2);
		style.WindowTitleAlign = ImVec2(0.5, 0.5);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
	}

	//Конфигурация ImGui под WinAPI
	if (ImGui_ImplWin32_Init(hwnd) != true) {
		std::string error = "Failed to init imgui for win32";
		throw std::runtime_error(error);
	}

	//Передача хэндлов Vulkan в ImGui
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
	
	//Конфигурация ImGui под Vulkan
	if (!ImGui_ImplVulkan_Init(&init_info, Engine::renderer.renderPass.GetRenderPass())){
		std::string error = "Failed to init imgui for Vulkan";
		throw std::runtime_error(error);
	}
	
	Engine::CommandBuffer oneTimeSubmitCommandBuffer;

	//Запись команд
	oneTimeSubmitCommandBuffer.AllocateCommandBuffer(Engine::renderer.device.Get(),Engine::renderer.commandPool.Get());
	oneTimeSubmitCommandBuffer.BeginCommandBuffer();
	
		//Загрузка шрифтов в ImGui
		if (!ImGui_ImplVulkan_CreateFontsTexture(oneTimeSubmitCommandBuffer.Get())) {
			std::string error = "Failed to load fonts to Imgui";
			throw std::runtime_error(error);
		}
	
	//Конец записи команд
	oneTimeSubmitCommandBuffer.EndCommandBuffer();
	oneTimeSubmitCommandBuffer.SubmitCommandBuffer(Engine::renderer.device.GetGraphicsQueue());
	oneTimeSubmitCommandBuffer.FreeCommandBuffer(Engine::renderer.device.Get(),
		Engine::renderer.commandPool.Get());
	
	//Освободить ресурсы для передачи данных
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

//Реализация интерфейса
void SceneEditor::DrawEditor(HWND hwnd, std::vector<Engine::Entity*>& Entities) {
	if (enableEditor) {
		
		//ImGui::ShowDemoWindow(&DemoWindowActive);//Демо ImGui
		
		ImGui::BeginMainMenuBar();//Менюбар

			MenubarHeight = ImGui::GetWindowHeight();

			//Вычитание из высоты 3D вьюпорта высоты менюбара
			Engine::renderer.rendererViewport.y = MenubarHeight;
			Engine::renderer.rendererViewport.height = Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight;

			//Меню: Файл
			if (ImGui::BeginMenu(u8"File")) {
				if (ImGui::MenuItem("New"))
				{
					SelectedItem_ID = -1;
					Engine::renderer.WaitForDrawFences();
					Engine::Globals::gScene->New();
					editorCamera.Reset();
					Engine::renderer.RebuildBuffers();
				}


				ImGui::MenuItem(u8"Open", "", &OpenFileDialog);
				if (OpenFileDialog){
					SelectedItem_ID = -1;
					std::string path = WinApiOpenDialog();
                    if (path!="")
                    {
                        spdlog::info("Loading...");
                        std::cout << path << std::endl;
						Engine::renderer.WaitForDrawFences();
                        Engine::Globals::gScene->Load(path);
					
						editorCamera.Reset();
                        spdlog::info("Done!");
                    }
					Engine::renderer.RebuildBuffers();
                    
					OpenFileDialog = false;
				}

				bool Save = false;
				bool SaveAs = false;

                ImGui::MenuItem("Save", "", &Save);
                if (Save) {
					if (Engine::Globals::gScene->GetScenePath() != "")
					{
                        spdlog::info("Saving...");
                        std::cout << Engine::Globals::gScene->GetScenePath() << std::endl;
						Engine::Globals::gScene->Save();
                        spdlog::info("Done!");
					}
					else {
						SaveAs = true;
					}
                }

				
				ImGui::MenuItem("Save As", "", &SaveAs);
				if (SaveAs){
					std::string path = WinApiSaveDialog();
					if (path != "")
					{
                        spdlog::info("Saving...");
                        std::cout << path << std::endl;
                        Engine::Globals::gScene->SaveAs(path);
                        spdlog::info("Done!");
					}
					
				}
				
				ImGui::MenuItem(u8"Exit", "ALT + F4", &CloseWindow);
			
				ImGui::EndMenu();
				if (CloseWindow) {
					SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
			}//Конец меню

			if (ImGui::BeginMenu(u8"Simulation")) {//Меню: начало симуляции
				ImGui::MenuItem(u8"Start", "", &StartButtonActive);

				if (StartButtonActive) {//Обработка нажатия кнопки "старт"

					Engine::Globals::gIsScenePlaying = true; //Начало сцены
					ResetPhysics = true; //Физика будет сброшена после остановки симуляции
							
				}
				else {
					Engine::Globals::gIsScenePlaying = false;
						
				
					if (ResetPhysics) {//Сброс параметров объектов
						for (size_t i = 0; i < Entities.size(); i++) {
							Entities.at(i)->Transform.ResetTransform();
							if (Entities.at(i)->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT) {
								Engine::GameObject* obj = (Engine::GameObject*)Entities.at(i);
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
						Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld->clearForces();
						ResetPhysics = false;
					}
				}
				ImGui::EndMenu();//Конец меню
			}
		
			if (ImGui::BeginMenu(u8"View")) { //Меню: "Вид"
				ImGui::MenuItem(u8"Hierarchy Panel", "", &ShowHierarchyPanel);
				ImGui::MenuItem(u8"Properties Panel", "", &ShowPropertiesPanel);
				ImGui::MenuItem(u8"Skybox", "", &Engine::Globals::states.showSkybox);
				ImGui::MenuItem(u8"Mesh", "", &Engine::Globals::states.showMeshes);
				ImGui::MenuItem(u8"Rigidbody Mesh", "", &Engine::Globals::states.showRigidbodyMeshes);
				ImGui::MenuItem(u8"Draw Shadows", "", &Engine::Globals::states.drawShadows);
				ImGui::MenuItem(u8"Use Scene Camera", "", &Engine::Globals::states.useSceneCamera);
				ImGui::EndMenu(); //Конец меню
			}
		ImGui::EndMainMenuBar(); //Конец менюбара
	
		//Панель иерархии
		if (ShowHierarchyPanel) {
			bool opened = true;
			ImGui::Begin(u8"Hierarchy", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

			//Размещение 3D вьюпорта справа от панели иерархии
			Engine::renderer.rendererViewport.x = ImGui::GetWindowWidth();
			Engine::renderer.rendererViewport.width = Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowWidth();

			//Учесть высоту менюбара при выводе панели иерархии
			ImGui::SetWindowPos(ImVec2(0, MenubarHeight));

			ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(),
				Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
			);

			if (ImGui::GetWindowWidth()>Engine::Globals::gWidth/2){
				ImGui::SetWindowSize(ImVec2((float)Engine::Globals::gWidth / 2,
					Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
				);
			}
				
			//Список объектов
			for (size_t i = 0; i < Entities.size(); i++) {
				if (ImGui::TreeNode(Entities.at(i)->GetName().c_str())) {
					ImGui::PushID(Entities.at(i)->GetID());
					ImGui::PopID();
					ImGui::TreePop();
				}

				if (ImGui::IsItemClicked()) {
					SelectedItem_ID = (int)i;
					spdlog::info("Object with ID {:08d} is selected", Entities.at(i)->GetID());

					//Получение ID выбранного объекта
					
					ShowPropertiesPanel = true;
				}
				if (ImGui::IsItemClicked(1))
				{
					SelectedItem_ID = (int)i;
					ImGui::OpenPopup("Popup");
				}
			}
			
			if (ImGui::BeginPopup("Popup")) {
				if(ImGui::MenuItem("Delete")) {
					if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT)
					{
                        delete Entities[SelectedItem_ID];
                        Entities.erase(Entities.begin() + SelectedItem_ID);
					}else
                    if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_DIRECTIONAL_LIGHT_OBJECT)
                    {
						for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->size(); i++)
						{
							if (Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->at(i)
								== ((Engine::DirectionalLightObject*)Entities[SelectedItem_ID])->pGetDirectionalLightUniformData())
							{

								Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->erase(
									Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->begin()+i
								);

							}
						}
                        delete Entities[SelectedItem_ID];
                        Entities.erase(Entities.begin() + SelectedItem_ID);
						
                    }
					else if(Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_POINTLIGHT_OBJECT)
					{
                        for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->size(); i++)
                        {
                            if (Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->at(i)
                                == ((Engine::PointLightObject*)Entities[SelectedItem_ID])->pGetPointLightUniformData())
                            {
                                Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->erase(
                                    Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->begin()+i
                                );

                            }
                        }
                        delete Entities[SelectedItem_ID];
                        Entities.erase(Entities.begin() + SelectedItem_ID);
					}
					else {

					}
					
					SelectedItem_ID = -1;
				}
				
                ImGui::EndPopup();
			}
			



			
			//TODO: добавление объекта
			if (ImGui::Button(u8"Add+")) {
				ImGui::OpenPopup("Select Object Type");
			}

            if (ImGui::BeginPopupModal("Select Object Type"))
            {
				ImGui::Text("Object types");
				ImGui::Separator();
                if (ImGui::MenuItem("Game Object"))
                {
                    Engine::GameObject* obj = new Engine::GameObject;
                    obj->AddComponent<Engine::Mesh>();
                    obj->pGetComponent<Engine::Mesh*>()->CreateMesh("");
                    obj->SetID((int)obj);
                    Entities.push_back(obj);
                }
                if (ImGui::MenuItem("Point Light"))
                {
                    Engine::PointLightObject* obj = new Engine::PointLightObject;
                    
                    obj->SetID((int)obj);
					Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->push_back(obj->pGetPointLightUniformData());
                    Entities.push_back(obj);
                }
                if (ImGui::MenuItem("Directional Light"))
                {

                    Engine::DirectionalLightObject* obj = new Engine::DirectionalLightObject;
					obj->pGetDirectionalLightUniformData()->lightColor = glm::vec3(1, 1, 1);
					obj->pGetDirectionalLightUniformData()->lightDirection = glm::vec3(1, -1, 1);
					Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->push_back(obj->pGetDirectionalLightUniformData());

                    obj->SetID((int)obj);
                    Entities.push_back(obj);
                }

                ImGui::EndPopup();
            }

			if (!opened) {
				ShowHierarchyPanel = false;
			}

			ImGui::End();
		}
		else {
			//Растяжение 3D вьюпорта если панель иерархии не отображена
			Engine::renderer.rendererViewport.x = 0;
			Engine::renderer.rendererViewport.width = static_cast<float>(Engine::renderer.swapchain.GetInfo().imageExtent.width);
		}//Конец панели иерархии
		


	
		if (ShowPropertiesPanel) {	//Панель свойств
			bool opened = true;

			ImGui::Begin(u8"Properties", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x, MenubarHeight));
            ImGui::SetWindowSize(
                ImVec2(Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x,
                    Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
            );

            

            //Расположение 3D вьюпорта слева от панели свойств
            Engine::renderer.rendererViewport.width -= Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x;

			if (SelectedItem_ID>=0){
                float min = -FLT_MAX;
                float max = FLT_MAX;
				
                switch (Entities.at(SelectedItem_ID)->GetEntityType()) {

				
                case Engine::ENTITY_TYPE_GAME_OBJECT://Панель свойств для объекта

                {//Выставление свойств слайдерами
                    glm::vec3 objectPos = Entities.at(SelectedItem_ID)->Transform.GetPosition();
                    glm::vec3 objectRotation = Entities.at(SelectedItem_ID)->Transform.GetEulerAngles();
                    glm::vec3 objectScale = Entities.at(SelectedItem_ID)->Transform.GetScaleValue();
                    glm::vec3 objectRigidbodyScale;
                    float mass;
                    float friction;
                    float restitution;
					Engine::DataTypes::Material_t mat;

					std::string name;
					name = Entities.at(SelectedItem_ID)->GetName();
					
                    InputTextCallback_UserData cb_user_data;
                    cb_user_data.Str = &name;
                    cb_user_data.ChainCallback = 0;
                    cb_user_data.ChainCallbackUserData = 0;

                    ImGui::InputText("Name", (char*)name.c_str(), name.capacity()+1, ImGuiInputTextFlags_CallbackResize,InputTextCallback,&cb_user_data);

                    if (ImGui::CollapsingHeader(u8"Translation")) {
						
                        ImGui::DragFloat3("Position", (float*)&objectPos, 1.0f, min, max);
                        ImGui::DragFloat3("Rotation", (float*)&objectRotation, 1.0f, min, max);
                        ImGui::DragFloat3("Scale", (float*)&objectScale, 1.0f, min, max);
                    }

                    if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>() != nullptr) {
                        objectRigidbodyScale = ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->pGetDebugMesh()->Transform.GetScaleValue();
						mass = ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->GetMass();
						friction = ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->GetFriction();
						restitution = ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->GetRestitution();

                        if (ImGui::CollapsingHeader(u8"Rigidbody")) {
                            ImGui::DragFloat3("Rigidbody Scale", (float*)&objectRigidbodyScale, 0.1f, min, max);
							ImGui::DragFloat("Mass", (float*)&mass, 0.1f, min, max);
							ImGui::DragFloat("Friction", (float*)&friction, 0.1f, min, max);
							ImGui::DragFloat("Restitution", (float*)&restitution, 0.1f, min, max);
                        }

                    }

                   if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>() != nullptr)
                   {
					   mat = ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->GetMaterial();
					   if (ImGui::CollapsingHeader(u8"Mesh")) {
                           ImGui::DragFloat("Shininess", (float*)&mat.shininess, 0.1f, min, max);
                           ImGui::DragFloat("Metallic", (float*)&mat.metallic, 0.1f, min, max);
                           ImGui::DragFloat("Roughness", (float*)&mat.roughness, 0.1f, min, max);
                           ImGui::DragFloat("Occlusion", (float*)&mat.ao, 0.1f, min, max);
                           


                           if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->pGetMeshPath() != "")
                           {
                               ImGui::TextWrapped(((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->pGetMeshPath().c_str());

                           }

                           if (ImGui::Button("Browse Mesh"))
                           {
                               std::string path = WinApiOpenDialog();
                               if (path != "")
                               {
                                   std::cout << path << std::endl;
                                   std::replace(path.begin(), path.end(), '\\', '/');
                                   ((Engine::GameObject*)Entities.at(SelectedItem_ID))->DeleteComponent<Engine::Mesh>();

                                   ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::Mesh>();
                                   ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->CreateMesh(path);
								     Engine::renderer.RebuildBuffers();
                               }
                           }



                           if (!((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->IsMaterialsFound())
                           {
                               ImGui::BulletText("MTL File Not Found!");
                           }
					   }                  
                   }

                    //Если сцена не проигрывается, то применить новые свойства
                    if (!Engine::Globals::gIsScenePlaying) {
						Entities.at(SelectedItem_ID)->Transform.Translate(objectPos);
						Entities.at(SelectedItem_ID)->Transform.Rotate(objectRotation);
						Entities.at(SelectedItem_ID)->Transform.Scale(objectScale);
						Entities.at(SelectedItem_ID)->SetName(name);
                        if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>() != nullptr) {
                            ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->SetRigidbodyScale(objectRigidbodyScale);
							((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->SetMass(mass);
							((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->SetFriction(friction);
							((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->SetRestitution(restitution);
                            ((Engine::GameObject*)Entities.at(SelectedItem_ID))->ApplyEntityTransformToRigidbody();
                        }
						if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>() != nullptr)
						{
							((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->SetMaterial(mat);
						}
                    }

					if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>() == nullptr)
					{
						if (ImGui::CollapsingHeader(u8"Mesh")) {
                            if (ImGui::Button("Create Mesh"))
                            {
                                ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::Mesh>();
                                ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->CreateMesh("");
								Engine::renderer.RebuildBuffers();

                            }
						}
					}
                    if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>() == nullptr)
                    {
						if (ImGui::CollapsingHeader(u8"Rigidbody")) {
                            if (ImGui::Button("Create Rigidbody"))
                            {
                                ImGui::OpenPopup("Choose Shapetype");


                            }
                            if (ImGui::BeginPopup("Choose Shapetype"))
                            {
                                if (ImGui::MenuItem("Cube"))
                                {
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::RigidBody>();
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->CreateRigidBody(
                                        Engine::RIGIDBODY_SHAPE_TYPE_CUBE,
                                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                                        SelectedItem_ID
                                    );
                                }
                                if (ImGui::MenuItem("Plane"))
                                {
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::RigidBody>();
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->CreateRigidBody(
                                        Engine::RIGIDBODY_SHAPE_TYPE_PLANE,
                                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                                        SelectedItem_ID
                                    );
                                }
                                if (ImGui::MenuItem("Sphere"))
                                {
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::RigidBody>();
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->CreateRigidBody(
                                        Engine::RIGIDBODY_SHAPE_TYPE_SPHERE,
                                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                                        SelectedItem_ID
                                    );
                                }
								Engine::renderer.RebuildBuffers();
                                ImGui::EndPopup();
                            }
						}
                    }
                }

                break;
		
                //Панель свойств для точечного источника света
                case Engine::ENTITY_TYPE_POINTLIGHT_OBJECT:
				{
                    std::string name;
                    name = Entities.at(SelectedItem_ID)->GetName();

                    InputTextCallback_UserData cb_user_data;
                    cb_user_data.Str = &name;
                    cb_user_data.ChainCallback = 0;
                    cb_user_data.ChainCallbackUserData = 0;

                    ImGui::InputText("Name", (char*)name.c_str(), name.capacity() + 1, ImGuiInputTextFlags_CallbackResize, InputTextCallback, &cb_user_data);

                    glm::vec3 objectPos = Entities.at(SelectedItem_ID)->Transform.GetPosition();

					Engine::DataTypes::PointLightAttributes_t pointLightAttributes = 
						*((Engine::PointLightObject*)Entities.at(SelectedItem_ID))->pGetPointLightUniformData();

                
                        ImGui::DragFloat3("Position", (float*)&objectPos, 1.0f, min, max);

                    

					if (ImGui::CollapsingHeader(u8"Light Params")) {

						ImGui::DragFloat3("Color", (float*)&pointLightAttributes.lightColor, 1.0f, min, max);

                        ImGui::DragFloat("Ambient", &pointLightAttributes.ambient);
                        ImGui::DragFloat("Diffuse", &pointLightAttributes.diffuse);
                        ImGui::DragFloat("Specular", &pointLightAttributes.specular);

                        ImGui::DragFloat("Constant", &pointLightAttributes.constant);
                        ImGui::DragFloat("Linear", &pointLightAttributes.linear);
                        ImGui::DragFloat("Quadrantic", &pointLightAttributes.quadrantic);
					}
                    

                    //Если сцена не проигрывается, то применить новые свойства
                    if (!Engine::Globals::gIsScenePlaying) {
						Entities.at(SelectedItem_ID)->SetName(name);
						Entities.at(SelectedItem_ID)->Transform.Translate(objectPos);
						*((Engine::PointLightObject*)Entities.at(SelectedItem_ID))->pGetPointLightUniformData() = pointLightAttributes;
                    }

                    break;
				}
                case Engine::ENTITY_TYPE_DIRECTIONAL_LIGHT_OBJECT:
				{
                    std::string name;
                    name = Entities.at(SelectedItem_ID)->GetName();

                    InputTextCallback_UserData cb_user_data;
                    cb_user_data.Str = &name;
                    cb_user_data.ChainCallback = 0;
                    cb_user_data.ChainCallbackUserData = 0;

                    ImGui::InputText("Name", (char*)name.c_str(), name.capacity() + 1, ImGuiInputTextFlags_CallbackResize, InputTextCallback, &cb_user_data);


                    Engine::DataTypes::DirectionalLightAttributes_t dirLightAttributes = *((Engine::DirectionalLightObject*)Entities.at(SelectedItem_ID))->pGetDirectionalLightUniformData();

                    ImGui::DragFloat3("Direction", (float*)&dirLightAttributes.lightDirection, 0.1f, min, max, "%.3f", 0.1f);
					if (ImGui::CollapsingHeader(u8"Light Params")) {
                        ImGui::DragFloat3("Color", (float*)&dirLightAttributes.lightColor, 0.1f, 0.0f, 255.f, "%.3f", 0.1f);
                        ImGui::DragFloat("Ambient", &dirLightAttributes.ambient);
                        ImGui::DragFloat("Diffuse", &dirLightAttributes.diffuse);
                        ImGui::DragFloat("Specular", &dirLightAttributes.specular);
					}

                    if (!Engine::Globals::gIsScenePlaying) {
						Entities.at(SelectedItem_ID)->SetName(name);
                        *((Engine::DirectionalLightObject*)Entities.at(SelectedItem_ID))->pGetDirectionalLightUniformData() = dirLightAttributes;
                    }

                    break;
				}
                default:
                    break;
                }
			}
			

			ImGui::End();
			if (!opened) {
				ShowPropertiesPanel = false;
			}
		}
		//Конец панели свойств

	}
	else {
		Engine::Globals::gIsScenePlaying = true;
	}
}


//Первоначальная настройка приложения
void Application::Init() {
	WindowWidth = 1366;
	WindowHeight = 768;

	Engine::Globals::gHeight = WindowHeight;
	Engine::Globals::gWidth = WindowWidth;
	sceneEditor.enableEditor = true;
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

	//Полный экран
	if (Engine::Globals::states.toggleFullscreen) {
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


	//Первоначальная настройка компонентов Vulkan
	Engine::renderer.initVulkan(hwnd, hInstance);

	
	Engine::Globals::gScene = new Engine::Scene;

	demo = (DemoFunc)GetProcAddress(GetModuleHandle(NULL), "DemoExe");

	demo(
		Engine::Globals::gScene,
		Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld
	);


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

//Главный цикл
void Application::Execute() {
	MSG msg = { };
	while (msg.message != WM_QUIT) {

		//Обработка событий
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		//Расчет времени итерации цикла
		{
			Time = GetTickCount();
			Engine::Globals::DeltaTime = Time - LastFrameTime;
			Engine::Globals::DeltaTime /= 100;
			LastFrameTime = Time;
		}

		//std::cout << Engine::Globals::DeltaTime << std::endl;
		
		

		if (Engine::Globals::gIsScenePlaying) {
			for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfEntities()->size(); i++) {

				//Применение физической модели к графической
				if (Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT) {
					((Engine::GameObject*)Engine::Globals::gScene->pGetVectorOfEntities()->at(i))->ApplyPhysicsToEntity();
				}

				Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->Update();
			}
			Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld->stepSimulation(floor(Engine::Globals::DeltaTime * 1000) / 1000);
		}

		

		if (ENABLE_IMGUI) {
			{//Формирование данных ImGUI для отрисовки кадра
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				sceneEditor.DrawEditor(hwnd, *Engine::Globals::gScene->pGetVectorOfEntities());

				ImGui::Render();
				ImguiDrawData = ImGui::GetDrawData();
				ImGui::EndFrame();
			}

		}


		if (!Engine::Globals::gIsScenePlaying)
		{
            //Отрисовка сцены c камерой редактора
            Engine::renderer.DrawScene(
                ImguiDrawData,
                Engine::Globals::gScene,
                sceneEditor.editorCamera
            );

			sceneEditor.editorCamera.Update();
		}
		else {
			if (Engine::Globals::states.useSceneCamera)
			{

				if (Engine::Globals::gScene->pGetActiveCamera()!=nullptr)
				{
                    Engine::renderer.DrawScene(
                        ImguiDrawData,
                        Engine::Globals::gScene,
                        *Engine::Globals::gScene->pGetActiveCamera()
                    );

                    Engine::Globals::gScene->UpdateActiveCamera();

                }
                else {
                    Engine::renderer.DrawScene(
                        ImguiDrawData,
                        Engine::Globals::gScene,
                        sceneEditor.editorCamera
                    );

                    sceneEditor.editorCamera.Update();
                }
			}
            else {
                Engine::renderer.DrawScene(
                    ImguiDrawData,
                    Engine::Globals::gScene,
                    sceneEditor.editorCamera
                );

                sceneEditor.editorCamera.Update();
            }
		}
		
	}
}

//<Освобождение ресурсов>
void Application::Clear() {
	//<Ожидание бездействия GPU>

	vkQueueWaitIdle(Engine::renderer.device.GetGraphicsQueue());
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
