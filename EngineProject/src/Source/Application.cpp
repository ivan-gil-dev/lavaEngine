#include "../Headers/Application.h"
#include <shobjidl.h>
#include <algorithm>
#include "../Headers/Renderer/Renderer.h"
#include <Shlwapi.h>
#include <thread>
#include "../Headers/Script.h"

void ResetEntities(std::vector<Engine::Entity*>* Entities) {
    for (size_t i = 0; i < Entities->size(); i++) {
        //����� ������� ��������//
        Entities->at(i)->Transform.ResetTransform();

        if (Entities->at(i)->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT) {
            Engine::GameObject* obj = (Engine::GameObject*)Entities->at(i);

            if (obj->pGetComponent<Engine::RigidBody*>() != nullptr) {
                //���������� ���������� ������������ � ����������� ����//
                obj->ApplyEntityTransformToRigidbody();

                btRigidBody* pRigidbody = obj->pGetComponent<Engine::RigidBody*>()->GetBulletRigidBody();

                //��������� ���������� ����������//
                if (pRigidbody->getMass() != 0) {
                    pRigidbody->clearForces();
                    pRigidbody->clearGravity();
                    pRigidbody->setLinearVelocity(btVector3(0, 0, 0));
                    pRigidbody->setAngularVelocity(btVector3(0, 0, 0));
                }
            }
        }
    }
}

//����������� ������� ��� ������ �� .exe
typedef void (*DemoFunc)(
    Engine::Scene*,
    btDynamicsWorld*
    );
DemoFunc demo;

//��������� ��������� ImGui
struct InputTextCallback_UserData
{
    std::string* Str;
    ImGuiInputTextCallback  ChainCallback;
    void* ChainCallbackUserData;
};

//string -> wstring
std::wstring StringToWideString(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

//��������� ������� ImGui ��� ��������� ����� ������
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

/*������ ��� �������� ����� json*/
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
                        // ��������� ���� � �����
                        // � ���������� ���� �� PWSTR -> std::string
                        std::wstring str(pszFilePath);
                        std::string path(str.begin(), str.end());

                        TCHAR buffer[MAX_PATH] = { 0 };
                        GetCurrentDirectory(MAX_PATH, buffer);

                        /*���� � ������������ � ������ ������ EXE �����*/
                        std::wstring buffer2 = buffer;
                        std::string currentDir(buffer2.begin(), buffer2.end());

                        /*��������� ���� � ����� ������������ EXE �����*/
                        char out[MAX_PATH] = "";
                        char* p = new char[path.length() + 1];
                        strcpy(p, path.c_str());
                        char* p2 = new char[currentDir.length() + 1];
                        strcpy(p2, currentDir.c_str());

                        PathRelativePathToA(out, p2, FILE_ATTRIBUTE_DIRECTORY,
                            p, FILE_ATTRIBUTE_NORMAL);

                        delete[]p;
                        delete[]p2;

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

/*������ ��� ���������� ����� json*/
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
                        /*PWSTR -> std::string*/
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

//������� ���������
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    //��������� ������� ������� � ImGui
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;

    switch (msg) {
    case WM_SYSKEYDOWN:

        if ((HIWORD(lparam) & 0x4000) == 0) {
            Engine::Globals::keyPressedEventHandler.ProcessKeyDownEvent(wparam, lparam);
        }

        //������ ���� �������������� ��� ������� ���������� ��������� ������
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
        //������������ ��������� ��� ��������� ������� ����

        if (Engine::renderer.device.Get() != VK_NULL_HANDLE) {
            Engine::renderer.recreateSwapchain();
        }

        return true;
        break;

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
        //��������� ������� �������
        Engine::Globals::cursorPosition.SetCursorPosFromWinAPI(LOWORD(lparam), HIWORD(lparam));
        return true;
        break;
    }
    return ::DefWindowProc(hwnd, msg, wparam, lparam);
}

/*��������� ��������� ���������*/
void SceneEditor::InitEditor(HWND hwnd) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    /*���������� ��������� ������� */
    ImGui::StyleColorsClassic();
    ImGuiStyle& style = ImGui::GetStyle(); {
        style.WindowRounding = 0.0f;
        style.WindowPadding = ImVec2(2, 2);
        style.FramePadding = ImVec2(2, 2);
        style.WindowTitleAlign = ImVec2(0.5, 0.5);
        style.WindowMenuButtonPosition = ImGuiDir_Right;
    }

    /*�������� ����������� ImGui � �������� ����*/
    if (ImGui_ImplWin32_Init(hwnd) != true) {
        std::string error = "Failed to init imgui for win32";
        throw std::runtime_error(error);
    }

    /*�������� ������������ ������������ Vulkan � ImGui*/
    ImGui_ImplVulkan_InitInfo init_info = {}; {
        init_info.Instance = Engine::renderer.instance.get();
        init_info.PhysicalDevice = Engine::renderer.physicalDevice.Get();
        init_info.Device = *Engine::renderer.device.PGet();
        init_info.QueueFamily = Engine::renderer.physicalDevice.GetQueueIndices().graphicsQueueIndex;
        init_info.Queue = Engine::renderer.device.GetGraphicsQueue();
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = Engine::renderer.descriptorPoolForImgui.Get();
        init_info.Allocator = nullptr;
        init_info.MinImageCount = Engine::renderer.swapchain.GetInfo().minImageCount;
        init_info.ImageCount = Engine::renderer.swapchain.GetInfo().minImageCount;
        init_info.CheckVkResultFn = imguiErrFunction;
    }

    //������������� ���������� ImGui ��� Vulkan
    if (!ImGui_ImplVulkan_Init(&init_info, Engine::renderer.renderPass.GetRenderPass())) {
        std::string error = "Failed to init imgui for Vulkan";
        throw std::runtime_error(error);
    }

    Engine::CommandBuffer oneTimeSubmitCommandBuffer;

    //������������� ������ ������ ��� �������� ������� � GPU
    oneTimeSubmitCommandBuffer.AllocateCommandBuffer(Engine::renderer.device.Get(), Engine::renderer.commandPool.Get());
    oneTimeSubmitCommandBuffer.BeginCommandBuffer();

    //�������� ������� � ����������
    if (!ImGui_ImplVulkan_CreateFontsTexture(oneTimeSubmitCommandBuffer.Get())) {
        std::string error = "Failed to load fonts to Imgui";
        throw std::runtime_error(error);
    }

    oneTimeSubmitCommandBuffer.EndCommandBuffer();
    oneTimeSubmitCommandBuffer.SubmitCommandBuffer(Engine::renderer.device.GetGraphicsQueue());
    oneTimeSubmitCommandBuffer.FreeCommandBuffer(Engine::renderer.device.Get(),
        Engine::renderer.commandPool.Get());

    /*	���������� ������� ��� �������� ������*/
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

/*��������� ������� ImGui � ������� ���������*/
void SceneEditor::UpdateEditor(HWND hwnd, std::vector<Engine::Entity*>& Entities) {
    if (enableEditor) {
        ImGui::ShowDemoWindow(&DemoWindowActive);

        /*������ ������ �������� ����*/
        ImGui::BeginMainMenuBar();
        /*��������� ������ ����*/
        MenubarHeight = ImGui::GetWindowHeight();
        /*�������� ������� ����� �� ������ ��������*/
        Engine::renderer.rendererViewport.y = MenubarHeight;
        /*��������� ������ �������� ����� �� ������ ��������*/
        Engine::renderer.rendererViewport.height = Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight;

        /*������ ������ �������� ���� File*/
        if (ImGui::BeginMenu(u8"File")) {
            /*������ ������ �������� ������ ���� New*/
            if (ImGui::MenuItem("New"))
            {
                /*����� ���������� �������*/
                SelectedItem_ID = -1;

                //�������� ����������� ������� �������� ������
                //��� ���������� ������ ���������
                Engine::renderer.WaitForDrawFences();

                /* �������� ������ ��� ������� ������ New*/
                std::thread thr(
                    &Engine::Scene::New_FromThread,
                    Engine::Globals::gScene,
                    std::ref(LoadingIsEnded)
                );

                /*����� ����� ��������� ���������� ������*/
                LoadingIsEnded = false;

                /*������ ������*/
                thr.detach();

                MSG msg = { };

                //�� ����, ��� ������� ����� �������� ���� LoadingIsEnded,
                //���� ����� �������� ���������
                while (!LoadingIsEnded)
                {
                    if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                /*����� ���������� ������ ���������*/
                editorCamera.Reset();

                /*��������� ���� ��� ���������� ������ ���������,
                                     ��� ��� ���-�� �������� � ����� ����������*/
                Engine::renderer.SetRebuildTrigger();
            }

            /*������ ������ �������� ������ ���� Open*/
            ImGui::MenuItem(u8"Open", "", &OpenFileDialog);
            if (OpenFileDialog) {
                //������� ������ �� ������
                SelectedItem_ID = -1;

                //����� (���� � ����� � JSON �������)
                std::string path = WinApiOpenDialog();

                //���� ���� � ����� �� ������
                if (path != "")
                {
                    spdlog::info("Loading...");
                    std::cout << path << std::endl;

                    //�������� ������������ �������
                    Engine::renderer.WaitForDrawFences();

                    //����� �������� �����
                    std::thread thr(
                        //������ �� ����� �������� �����
                        &Engine::Scene::Load_FromThread,
                        //��������� �� �����
                        Engine::Globals::gScene,
                        //���� � �����
                        path,
                        //������ �� atomic<bool>
                        std::ref(LoadingIsEnded)
                    );

                    //����� ���� LoadingIsEnded//
                    LoadingIsEnded = false;

                    //������ �������� ������//
                    thr.detach();

                    MSG msg = { };
                    //���� ����� ����������� ���� ����� �������� ���������//
                    while (!LoadingIsEnded)
                    {
                        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }

                    //����� ������ �����//
                    editorCamera.Reset();

                    spdlog::info("Done!");
                }

                //����������� swapchain//
                Engine::renderer.recreateSwapchain();

                //����������� ������ ������ ������ �����//
                Engine::renderer.SetRebuildTrigger();

                //������ ������...
                OpenFileDialog = false;
            }

            bool Save = false;
            bool SaveAs = false;

            //����� ���� "Save"
            ImGui::MenuItem("Save", "", &Save);
            if (Save) {
                if (Engine::Globals::gScene->GetScenePath() != "")
                {
                    spdlog::info("Saving...");

                    //��������� ���� � ������� �����
                    std::cout << Engine::Globals::gScene->GetScenePath() << std::endl;

                    std::thread thr(
                        //������ �� ����� ����������
                        &Engine::Scene::Save_FromThread,
                        //��������� �� �����
                        Engine::Globals::gScene,
                        //������ �� atomic<bool>
                        std::ref(LoadingIsEnded)
                    );

                    thr.join();

                    spdlog::info("Done!");
                }
                else {
                    SaveAs = true;
                }
            }

            //����� ���� "SaveAs"...
            ImGui::MenuItem("Save As", "", &SaveAs);
            if (SaveAs) {
                //����� (���������� �����)
                std::string path = WinApiSaveDialog();
                if (path != "")
                {
                    spdlog::info("Saving...");
                    std::cout << path << std::endl;

                    //����� ���������� �����
                    std::thread thr(
                        //������ �� �����
                        &Engine::Scene::SaveAs_FromThread,
                        //��������� �� �����
                        Engine::Globals::gScene,
                        //���� � �����
                        path,
                        std::ref(LoadingIsEnded)
                    );

                    thr.join();

                    spdlog::info("Done!");
                }
            }

            ImGui::MenuItem(u8"Exit", "ALT + F4", &CloseWindow);
            ImGui::EndMenu();

            if (CloseWindow) {
                SendMessage(hwnd, WM_CLOSE, 0, 0);
            }
        }

        if (ImGui::BeginMenu(u8"Simulation")) {
            ImGui::MenuItem(u8"Start", "", &StartButtonActive);

            if (StartButtonActive) {//��������� ������� ������ "�����"//
                Engine::Globals::gIsScenePlaying = true; //����� �������������//
                ResetPhysics = true; //������ ����� �������� ����� ��������� ���������//
            }
            else {
                Engine::Globals::gIsScenePlaying = false;

                if (ResetPhysics) {
                    //����� ���������� ��������//
                    ResetEntities(Engine::Globals::gScene->pGetVectorOfEntities());
                    Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld->clearForces();
                    ResetPhysics = false;
                }
            }
            ImGui::EndMenu();//����� ����//
        }

        if (ImGui::BeginMenu(u8"View")) { //����: "���"//
            ImGui::MenuItem(u8"Hierarchy Panel", "", &ShowHierarchyPanel);//�������� ������ ��������//
            ImGui::MenuItem(u8"Properties Panel", "", &ShowPropertiesPanel);//�������� ������ �������//
            ImGui::MenuItem(u8"Skybox", "", &Engine::Globals::states.showSkybox);//�������� ��������//
            ImGui::MenuItem(u8"Mesh", "", &Engine::Globals::states.showMeshes);//�������� 3D ������//
            ImGui::MenuItem(u8"Rigidbody Mesh", "", &Engine::Globals::states.showRigidbodyMeshes);//�������� ������ ����������� �������//
            ImGui::MenuItem(u8"Draw Shadows", "", &Engine::Globals::states.drawShadows);//�������� ����//
            ImGui::MenuItem(u8"Use Scene Camera", "", &Engine::Globals::states.useSceneCamera);//������������ �������� ������ �����//
            ImGui::EndMenu(); //����� ����//
        }
        ImGui::EndMainMenuBar(); //����� ��������//

        //������ ��������//
        if (ShowHierarchyPanel) {
            bool opened = true;
            //������ ������ ���������� ������ ��������//
            ImGui::Begin(u8"Hierarchy", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            //�������� ������� ������ �� ������ ������ ��������//
            Engine::renderer.rendererViewport.x = ImGui::GetWindowWidth();

            //������ �������� ��������� �� ������ ���� ��������//
            Engine::renderer.rendererViewport.width = Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowWidth();

            //������ ������ �������� ��� ������ ������ ��������//
            ImGui::SetWindowPos(ImVec2(0, MenubarHeight));
            ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(),
                Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
            );

            //������ ��������//
            for (size_t i = 0; i < Entities.size(); i++) {
                //����� ������//
                if (ImGui::TreeNode(Entities.at(i)->GetName().c_str())) {
                    //��� ������� �� ������ ������������� ID ������� ���� �����//
                    ImGui::PushID(Entities.at(i)->GetID());
                    ImGui::PopID();
                    ImGui::TreePop();
                }

                //��������� ������� �� ����� ������//
                if (ImGui::IsItemClicked()) {
                    //������ i-� ������//
                    SelectedItem_ID = (int)i;
                    spdlog::info("Object with ID {:08d} is selected", Entities.at(i)->GetID());

                    //�������� ������ �������//
                    ShowPropertiesPanel = true;
                }

                //��������� ������� ������ ������ ���� �� ����� ������//
                if (ImGui::IsItemClicked(1))
                {
                    SelectedItem_ID = (int)i;
                    //����� ������������ ���� Popup//
                    ImGui::OpenPopup("Popup");
                }
            }

            if (SelectedItem_ID > Entities.size() - 1)
            {
                SelectedItem_ID = -1;
            }

            if (ImGui::BeginPopup("Popup")) {
                //�������� ���������
                if (ImGui::MenuItem("Duplicate")) {
                    if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT)
                    {
                        Engine::GameObject* obj = new Engine::GameObject;
                        obj->SetName(Entities[SelectedItem_ID]->GetName());
                        obj->SetID(reinterpret_cast<uint64_t>(reinterpret_cast<int*>(obj)));
                        obj->Transform = Entities[SelectedItem_ID]->Transform;
                        if (((Engine::GameObject*)Entities[SelectedItem_ID])->pGetComponent<Engine::Mesh*>() != nullptr)
                        {
                            obj->AddComponent<Engine::Mesh>();
                            Engine::Mesh* m = obj->pGetComponent<Engine::Mesh*>();

                            std::thread thr(
                                &Engine::Mesh::CreateMesh_FromThread,
                                m,
                                ((Engine::GameObject*)Entities[SelectedItem_ID])->pGetComponent<Engine::Mesh*>()->pGetMeshPath(),
                                std::ref(LoadingIsEnded)
                            );

                            LoadingIsEnded = false;

                            //������ �������� ������//
                            thr.detach();

                            MSG msg = { };
                            while (!LoadingIsEnded)
                            {
                                if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                                    TranslateMessage(&msg);
                                    DispatchMessage(&msg);
                                }
                            }

                            m->SetMaterial(((Engine::GameObject*)Entities[SelectedItem_ID])->pGetComponent<Engine::Mesh*>()->GetMaterial());
                        }
                        if (((Engine::GameObject*)Entities[SelectedItem_ID])->pGetComponent<Engine::RigidBody*>() != nullptr)
                        {
                            obj->AddComponent<Engine::RigidBody>();
                            Engine::RigidBody* r = obj->pGetComponent<Engine::RigidBody*>();
                            r->CreateRigidBody(
                                ((Engine::GameObject*)Entities[SelectedItem_ID])->pGetComponent<Engine::RigidBody*>()->GetShapeType(),
                                Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                                obj->GetID()
                            );
                            r->SetRigidbodyScale(((Engine::GameObject*)Entities[SelectedItem_ID])->pGetComponent<Engine::RigidBody*>()->GetRigidbodyScale());
                            r->SetMass(((Engine::GameObject*)Entities[SelectedItem_ID])->pGetComponent<Engine::RigidBody*>()->GetMass());
                            r->SetFriction(((Engine::GameObject*)Entities[SelectedItem_ID])->pGetComponent<Engine::RigidBody*>()->GetFriction());
                        }

                        Entities.push_back(obj);
                    }
                }
                //������ Delete//
                if (ImGui::MenuItem("Delete")) {
                    //�������� ������������ ������� ��� ���������//
                    Engine::renderer.WaitForDrawFences();

                    //�������� ���������� �� ������� ��� ������� ���� �������//
                    if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT)
                    {
                        //������������ ������//
                        delete Entities[SelectedItem_ID];
                        //�������� �� �������//
                        Entities.erase(Entities.begin() + SelectedItem_ID);
                    }
                    else
                        if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_DIRECTIONAL_LIGHT_OBJECT)
                        {
                            /*����� ���������� i-�� ��������� ����� � ������� ���������*/
                            for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->size(); i++)
                            {
                                /*���� ��������� �������, �� ��� ��������� �� ������� ���������*/
                                if (Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->at(i)
                                    == ((Engine::DirectionalLightObject*)Entities[SelectedItem_ID])->pGetDirectionalLightUniformData())
                                {
                                    Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->erase(
                                        Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->begin() + i
                                    );
                                }
                            }
                            //������������ ������//
                            delete Entities[SelectedItem_ID];
                            //�������� �� ������� ���������//
                            Entities.erase(Entities.begin() + SelectedItem_ID);
                        }
                        else if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_POINTLIGHT_OBJECT)
                        {
                            /*����� ���������� i-�� ��������� ����� � ������� ���������*/
                            for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->size(); i++)
                            {
                                //���� ��������� �������, �� ��� ��������� �� ������� ���������//
                                if (Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->at(i)
                                    == ((Engine::PointLightObject*)Entities[SelectedItem_ID])->pGetPointLightUniformData())
                                {
                                    Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->erase(
                                        Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->begin() + i
                                    );
                                }
                            }
                            //������������ ������//
                            delete Entities[SelectedItem_ID];
                            //�������� �� ������� ���������//
                            Entities.erase(Entities.begin() + SelectedItem_ID);
                        }
                        else {
                        }
                    //������� ������ �� ������//
                    SelectedItem_ID = -1;
                }

                ImGui::EndPopup();
            }

            //���������� �������//
            if (ImGui::Button(u8"Add+")) {
                //����� ������������ ���� "Select Object Type"//
                ImGui::OpenPopup("Select Object Type");
            }

            if (ImGui::BeginPopupModal("Select Object Type"))
            {
                ImGui::Text("Object types");
                ImGui::Separator();
                //������, ��� ������� �� ������� ��������� ������� ������//
                if (ImGui::MenuItem("Game Object"))
                {
                    Engine::GameObject* obj = new Engine::GameObject;
                    //obj->AddComponent<Engine::Mesh>();
                    //obj->pGetComponent<Engine::Mesh*>()->CreateMesh("");
                    obj->SetID(reinterpret_cast<uint64_t>(reinterpret_cast<int*>(obj)));
                    Entities.push_back(obj);
                }

                //������, ��� ������� �� ������� ��������� �������� �������� �����//
                if (ImGui::MenuItem("Point Light"))
                {
                    Engine::PointLightObject* obj = new Engine::PointLightObject;

                    obj->SetID(reinterpret_cast<uint64_t>(reinterpret_cast<int*>(obj)));
                    Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->push_back(obj->pGetPointLightUniformData());
                    Entities.push_back(obj);
                }

                //������, ��� ������� �� ������� ��������� ������������ �������� �����//
                if (ImGui::MenuItem("Directional Light"))
                {
                    Engine::DirectionalLightObject* obj = new Engine::DirectionalLightObject;
                    obj->pGetDirectionalLightUniformData()->lightColor = glm::vec3(1, 1, 1);
                    obj->pGetDirectionalLightUniformData()->lightDirection = glm::vec3(1, -1, 1);
                    Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->push_back(obj->pGetDirectionalLightUniformData());

                    obj->SetID(reinterpret_cast<uint64_t>(reinterpret_cast<int*>(obj)));
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
            //���������� 3D �������� ���� ������ �������� �� ����������
            Engine::renderer.rendererViewport.x = 0;
            Engine::renderer.rendererViewport.width = static_cast<float>(Engine::renderer.swapchain.GetInfo().imageExtent.width);
        }//����� ������ ��������

        //������ �������//
        if (ShowPropertiesPanel) {
            bool opened = true;

            //������ ������ ���������� ������ �������//
            ImGui::Begin(u8"Properties", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x, MenubarHeight));
            ImGui::SetWindowSize(
                ImVec2(Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x,
                    Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
            );

            //������������ 3D �������� ����� �� ������ �������//
            Engine::renderer.rendererViewport.width -= Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x;

            //���� ������� ������ ������//
            if (SelectedItem_ID >= 0) {
                //����������� �������� ���� ������ float//
                float min = -FLT_MAX;
                //������������ �������� ���� ������ float//
                float max = FLT_MAX;

                //����� ������� � ����������� �� ���� �������//
                switch (Entities.at(SelectedItem_ID)->GetEntityType()) {
                    //����� ������� ��� �������� �������//
                case Engine::ENTITY_TYPE_GAME_OBJECT:
                {
                    //��������� ������� ��������//
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

                    ImGui::InputText("Name", (char*)name.c_str(), name.capacity() + 1, ImGuiInputTextFlags_CallbackResize, InputTextCallback, &cb_user_data);

                    //�������� ����������� ��������//
                    if (ImGui::CollapsingHeader(u8"Translation")) {
                        ImGui::DragFloat3("Position", (float*)&objectPos, 1.0f, min, max);
                        ImGui::DragFloat3("Rotation", (float*)&objectRotation, 1.0f, min, max);
                        ImGui::DragFloat3("Scale", (float*)&objectScale, 1.0f, min, max);
                    }

                    //�������� ����������� ����//
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

                    //�������� ���������� ������//
                    if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>() != nullptr)
                    {
                        //��������� ��������� ������//
                        mat = ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->GetMaterial();
                        if (ImGui::CollapsingHeader(u8"Mesh")) {
                            //��������� ���������� �������//
                            ImGui::DragFloat("Shininess", (float*)&mat.shininess, 0.1f, min, max);
                            ImGui::DragFloat("Metallic", (float*)&mat.metallic, 0.1f, min, max);
                            ImGui::DragFloat("Roughness", (float*)&mat.roughness, 0.1f, min, max);
                            ImGui::DragFloat("Occlusion", (float*)&mat.ao, 0.1f, min, max);

                            //����� ���� � ����������� �������//
                            if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->pGetMeshPath() != "")
                            {
                                ImGui::TextWrapped(((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->pGetMeshPath().c_str());
                            }

                            //����� (�������� ����)//
                            if (ImGui::Button("Browse Mesh"))
                            {
                                //�������� ���� � ����������� ������� ����� ���������� ����//
                                std::string path = WinApiOpenDialog();
                                if (path != "")
                                {
                                    std::cout << path << std::endl;
                                    std::replace(path.begin(), path.end(), '\\', '/');

                                    Engine::renderer.WaitForDrawFences();

                                    //�������� ����������� ���������� ������ (����)
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->DeleteComponent<Engine::Mesh>();

                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::Mesh>();

                                    //�������� ������ �� ���������� ����//
                                    std::thread thr(
                                        //������ �� ����� �������� ������//
                                        &Engine::Mesh::CreateMesh_FromThread,
                                        //��������� �� ���������� ������//
                                        ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>(),
                                        //���� � �������//
                                        path,
                                        //������ �� atomic<bool>//
                                        std::ref(LoadingIsEnded)
                                    );

                                    LoadingIsEnded = false;

                                    //������ �������� ������//
                                    thr.detach();

                                    MSG msg = { };
                                    //���� ������� ����� ����� �����������//
                                    //���� ����� �������� ���������//
                                    while (!LoadingIsEnded)
                                    {
                                        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                                            TranslateMessage(&msg);
                                            DispatchMessage(&msg);
                                        }
                                    }

                                    //����������� swapchain//
                                    Engine::renderer.recreateSwapchain();
                                    //����������� ������//
                                    Engine::renderer.SetRebuildTrigger();
                                }
                            }

                            if (!((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->IsMaterialsFound())
                            {
                                ImGui::BulletText("MTL File Not Found!");
                            }
                        }
                    }

                    //���� ����� �� �������������, �� ��������� ����� ��������//
                    if (!Engine::Globals::gIsScenePlaying) {
                        Entities.at(SelectedItem_ID)->Transform.SetTranslation(objectPos);
                        Entities.at(SelectedItem_ID)->Transform.SetRotation(objectRotation);
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
                        //������ �������� ���������� ������, ���� �������� �� �������� ������//
                        if (ImGui::CollapsingHeader(u8"Mesh")) {
                            if (ImGui::Button("Create Mesh"))
                            {
                                ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::Mesh>();

                                WaitMessage();

                                std::thread thr(
                                    &Engine::Mesh::CreateMesh,
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>(),
                                    "");

                                thr.join();

                                Engine::renderer.SetRebuildTrigger();
                            }
                        }
                    }
                    if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>() == nullptr)
                    {
                        //����� ����� ����������� ���� -> �������� ����������� ����
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
                                        reinterpret_cast<uint64_t>(reinterpret_cast<int*>(Entities[SelectedItem_ID]))
                                    );

                                    std::cout << reinterpret_cast<uint64_t>(reinterpret_cast<int*>(Entities[SelectedItem_ID])) << std::endl;
                                    std::cout << ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->GetBulletRigidBody()->GetEntityId() << std::endl;
                                }
                                if (ImGui::MenuItem("Plane"))
                                {
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::RigidBody>();
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->CreateRigidBody(
                                        Engine::RIGIDBODY_SHAPE_TYPE_PLANE,
                                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                                        reinterpret_cast<uint64_t>(reinterpret_cast<int*>(Entities[SelectedItem_ID]))
                                    );
                                }
                                if (ImGui::MenuItem("Sphere"))
                                {
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::RigidBody>();
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->CreateRigidBody(
                                        Engine::RIGIDBODY_SHAPE_TYPE_SPHERE,
                                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                                        reinterpret_cast<uint64_t>(reinterpret_cast<int*>(Entities[SelectedItem_ID]))
                                    );
                                }
                                if (ImGui::MenuItem("Mesh"))
                                {
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::RigidBody>();
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->CreateRigidBody(
                                        ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>(),
                                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                                        reinterpret_cast<uint64_t>(reinterpret_cast<int*>(Entities[SelectedItem_ID])),
                                        false
                                    );
                                }
                                if (ImGui::MenuItem("Static Mesh"))
                                {
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::RigidBody>();
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->CreateRigidBody(
                                        ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>(),
                                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                                        reinterpret_cast<uint64_t>(reinterpret_cast<int*>(Entities[SelectedItem_ID])),
                                        true
                                    );
                                }

                                Engine::renderer.SetRebuildTrigger();
                                ImGui::EndPopup();
                            }
                        }
                    }

                    //���������� ��������
                    if (ImGui::CollapsingHeader(u8"Script")) {
                        ImGui::TextWrapped((Entities.at(SelectedItem_ID))->pGetScript()->GetScriptPath().c_str());
                        if (ImGui::Button("Browse Script"))
                        {
                            std::string path = WinApiOpenDialog();
                            if (path != "")
                            {
                                Entities.at(SelectedItem_ID)->pGetScript()->SetScriptPath(path);
                            }
                        }
                        if (Entities.at(SelectedItem_ID)->pGetScript()->GetScriptPath() != "")
                        {
                            if (ImGui::Button("Open Script"))
                            {
                                std::string scriptPath = Entities.at(SelectedItem_ID)->pGetScript()->GetScriptPath();
                                std::replace(scriptPath.begin(), scriptPath.end(), '/', '\\');
                                std::wstring scr = StringToWideString(scriptPath);
                                LPCWSTR s = scr.c_str();

                                SHELLEXECUTEINFO ShExecInfo;
                                ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
                                ShExecInfo.fMask = NULL;
                                ShExecInfo.hwnd = NULL;
                                ShExecInfo.lpVerb = NULL;
                                ShExecInfo.lpFile = s;
                                ShExecInfo.lpParameters = NULL;
                                ShExecInfo.lpDirectory = NULL;
                                ShExecInfo.nShow = SW_MAXIMIZE;
                                ShExecInfo.hInstApp = NULL;

                                int r = ShellExecuteEx(&ShExecInfo);
                            }
                        }
                    }
                }

                break;

                //������ ������� ��� ��������� ��������� �����//
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

                    //��������� ��������� ��������� �����//
                    if (ImGui::CollapsingHeader(u8"Light Params")) {
                        ImGui::DragFloat3("Color", (float*)&pointLightAttributes.lightColor, 1.0f, min, max);

                        ImGui::DragFloat("Ambient", &pointLightAttributes.ambient);
                        ImGui::DragFloat("Diffuse", &pointLightAttributes.diffuse);
                        ImGui::DragFloat("Specular", &pointLightAttributes.specular);

                        ImGui::DragFloat("Constant", &pointLightAttributes.constant);
                        ImGui::DragFloat("Linear", &pointLightAttributes.linear);
                        ImGui::DragFloat("Quadrantic", &pointLightAttributes.quadrantic);
                    }

                    //���� ����� �� �������������, �� ��������� ����� ��������//
                    if (!Engine::Globals::gIsScenePlaying) {
                        Entities.at(SelectedItem_ID)->SetName(name);
                        Entities.at(SelectedItem_ID)->Transform.SetTranslation(objectPos);
                        *((Engine::PointLightObject*)Entities.at(SelectedItem_ID))->pGetPointLightUniformData() = pointLightAttributes;
                    }

                    //���������� ��������
                    if (ImGui::CollapsingHeader(u8"Script")) {
                        ImGui::TextWrapped((Entities.at(SelectedItem_ID))->pGetScript()->GetScriptPath().c_str());
                        if (ImGui::Button("Browse Script"))
                        {
                            std::string path = WinApiOpenDialog();
                            if (path != "")
                            {
                                Entities.at(SelectedItem_ID)->pGetScript()->SetScriptPath(path);
                            }
                        }
                        if (Entities.at(SelectedItem_ID)->pGetScript()->GetScriptPath() != "")
                        {
                            if (ImGui::Button("Open Script"))
                            {
                                std::string scriptPath = Entities.at(SelectedItem_ID)->pGetScript()->GetScriptPath();
                                std::replace(scriptPath.begin(), scriptPath.end(), '/', '\\');
                                std::wstring scr = StringToWideString(scriptPath);
                                LPCWSTR s = scr.c_str();

                                SHELLEXECUTEINFO ShExecInfo;
                                ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
                                ShExecInfo.fMask = NULL;
                                ShExecInfo.hwnd = NULL;
                                ShExecInfo.lpVerb = NULL;
                                ShExecInfo.lpFile = s;
                                ShExecInfo.lpParameters = NULL;
                                ShExecInfo.lpDirectory = NULL;
                                ShExecInfo.nShow = SW_MAXIMIZE;
                                ShExecInfo.hInstApp = NULL;

                                int r = ShellExecuteEx(&ShExecInfo);
                            }
                        }
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

                    //��������� ��������� ��������� �����//
                    ImGui::DragFloat3("Direction", (float*)&dirLightAttributes.lightDirection, 0.1f, min, max, "%.3f", 0.1f);
                    if (ImGui::CollapsingHeader(u8"Light Params")) {
                        ImGui::DragFloat3("Color", (float*)&dirLightAttributes.lightColor, 0.1f, 0.0f, 255.f, "%.3f", 0.1f);
                        ImGui::DragFloat("Ambient", &dirLightAttributes.ambient);
                        ImGui::DragFloat("Diffuse", &dirLightAttributes.diffuse);
                        ImGui::DragFloat("Specular", &dirLightAttributes.specular);
                    }

                    //���� ����� �� �������������, �� ��������� ����� ��������//
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
        //����� ������ �������
    }
    else {
        //���� �������� �� ������������ �� ����� �������������//
        Engine::Globals::gIsScenePlaying = true;
    }
}

void RegisterClasses() {
    luabridge::getGlobalNamespace(Engine::Lua::gLuaState)
        .beginClass<Engine::Entity>("Entity")
        .addFunction("Test", &Engine::Entity::lua_Test)
        .addFunction("MoveEntity", &Engine::Entity::lua_MoveEntity)
        .addFunction("RotateEntity", &Engine::Entity::lua_RotateEntity)
        .addFunction("TranslateEntity", &Engine::Entity::lua_TranslateEntity)
        .addFunction("TriggerDestroy", &Engine::Entity::TriggerDestroy)
        .addFunction("GetName", &Engine::Entity::GetName)
        .endClass()
        .beginClass<Engine::GameObject>("GameObject")
        .addFunction("Test", &Engine::GameObject::lua_Test)
        .addFunction("MoveEntity", &Engine::GameObject::lua_MoveEntity)
        .addFunction("RotateEntity", &Engine::GameObject::lua_RotateEntity)
        .addFunction("TranslateEntity", &Engine::GameObject::lua_TranslateEntity)
        .addFunction("TriggerDestroy", &Engine::GameObject::TriggerDestroy)
        .addFunction("GetName", &Engine::Entity::GetName)
        .addFunction("ApplyEntityTransformToRigidbody", &Engine::GameObject::ApplyEntityTransformToRigidbody)
        .endClass()
        .getGlobalNamespace(Engine::Lua::gLuaState);
}

void ProcessCollisions() {
    struct ContactObjects
    {
        Engine::Entity* obj1;
        Engine::Entity* obj2;
        ContactObjects(Engine::Entity* ent1, Engine::Entity* ent2) {
            obj1 = ent1;
            obj2 = ent2;
        }
    };

    std::vector<ContactObjects> contactObjects;

    btDynamicsWorld* world = Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld;
    int numManifolds = world->getDispatcher()->getNumManifolds();

    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject* obA = contactManifold->getBody0();
        const btCollisionObject* obB = contactManifold->getBody1();

        int numContacts = contactManifold->getNumContacts();
        for (int j = 0; j < numContacts; j++)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if (pt.getDistance() < 0.f)
            {
                const btVector3& ptA = pt.getPositionWorldOnA();
                const btVector3& ptB = pt.getPositionWorldOnB();
                const btVector3& normalOnB = pt.m_normalWorldOnB;
            }
        }

        Engine::Entity* entityA = reinterpret_cast<Engine::Entity*>(((Engine::Engine_Bullet3Rigidbody*)obA)->GetEntityId());
        Engine::Entity* entityB = reinterpret_cast<Engine::Entity*>(((Engine::Engine_Bullet3Rigidbody*)obB)->GetEntityId());

        contactObjects.push_back(ContactObjects(entityA, entityB));
    }

    for (std::vector<ContactObjects>::iterator i = contactObjects.begin(); i != contactObjects.end(); i++)
    {
        (*i).obj1->pGetScript()->doScriptOnCollision((Engine::GameObject*)(*i).obj1, (Engine::GameObject*)(*i).obj2);
        /*if ((*i).obj1->GetName() == "Sphere" && (*i).obj2->GetName() == "box")
        {
            (*i).obj2->DestroyEntity = true;
        }*/
    }

    std::vector<Engine::Entity*>* Entities = Engine::Globals::gScene->pGetVectorOfEntities();

    for (std::vector<Engine::Entity*>::iterator i = Entities->begin(); i != Entities->end(); i++)
    {
        if ((*i)->IsDestroyTriggered()) {
            Engine::renderer.WaitForDrawFences();
            delete (*i);
            Entities->erase(i--);
        }
    }
}

//�������������//
void Application::Init() {
    Engine::Lua::gLuaState = luaL_newstate();
    luaL_openlibs(Engine::Lua::gLuaState);

    luabridge::setGlobal(Engine::Lua::gLuaState, 0, "DeltaTime");
    RegisterClasses();

    Engine::Lua::Script scr;
    scr.SetScriptPath("CoreAssets/settings.lua");
    int r = scr.doScript();

    if (r == LUA_OK)
    {
        WindowWidth = (int)scr.getVar<lua_Integer>("windowWidth");
        WindowHeight = (int)scr.getVar<lua_Integer>("windowHeight");
    }
    else {
        WindowWidth = 1366;
        WindowHeight = 768;
    }

    Engine::Globals::gHeight = WindowHeight;
    Engine::Globals::gWidth = WindowWidth;

    //���������� �������� ������//
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

    if (Engine::Globals::states.toggleFullscreen) {
        //�������� ���� � ������������� ������//
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
        //�������� ���� � ������� ������//
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

    //::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    //�������������� ��������� ����������� Vulkan//
    Engine::renderer.initVulkan(hwnd, hInstance);

    //��������� ������ ��� �����//
    Engine::Globals::gScene = new Engine::Scene;

    //��������� ������ ��������� �� ������� �������//
    demo = (DemoFunc)GetProcAddress(GetModuleHandle(NULL), "DemoExe");

    //�������� ����� �� ������� �������//
    demo(
        Engine::Globals::gScene,
        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld
    );

    sceneEditor.enableEditor = Engine::Globals::enableEditor;

    if (ENABLE_IMGUI) {
        sceneEditor.InitEditor(hwnd);
    }
    else {
        Engine::Globals::gIsScenePlaying = true;
    }

    //Time = 0;
    //LastFrameTime = 0;
    Engine::Globals::DeltaTime = 0;
}

//������� ����//
void Application::Execute() {
    MSG msg = { };

    ResetEntities(Engine::Globals::gScene->pGetVectorOfEntities());

    while (msg.message != WM_QUIT) {
        //��������� ���������//
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        //������ ������� �������� �����//
        Time = std::chrono::high_resolution_clock::now();
        Engine::Globals::DeltaTime = (
            std::chrono::duration_cast<std::chrono::microseconds>(Time.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::microseconds>(LastFrameTime.time_since_epoch()).count()
            );
        LastFrameTime = Time;

        //������� � �������
        Engine::Globals::DeltaTime /= 1000000;
        luabridge::setGlobal(Engine::Lua::gLuaState, Engine::Globals::DeltaTime, "DeltaTime");

        if (Engine::Globals::gIsScenePlaying) {
            //��� ��������� ���������� ������ �����������//
            float step = (float)Engine::Globals::DeltaTime;
            Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld->stepSimulation(step, 0);
            ProcessCollisions();
        }

        if (ENABLE_IMGUI) {
            {
                //���������� ������ ����� ImGui//
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                //������ ������ �����//
                sceneEditor.UpdateEditor(hwnd, *Engine::Globals::gScene->pGetVectorOfEntities());

                //���������� ������ ��� �������� � GPU//
                ImGui::Render();
                ImguiDrawData = ImGui::GetDrawData();
                ImGui::EndFrame();
            }
        }

        Engine::Globals::DeltaTime *= 10;

        if (!Engine::Globals::gIsScenePlaying)
        {
            sceneEditor.editorCamera.Update();

            std::thread drawWithEditorCamera(
                &Engine::Renderer::DrawScene,
                &Engine::renderer,
                ImguiDrawData,
                Engine::Globals::gScene,
                sceneEditor.editorCamera
            );

            drawWithEditorCamera.join();
        }
        else
        {
            for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfEntities()->size(); i++)
            {
                //���������� ���������� ������ � �����������//
                if (Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT)
                {
                    ((Engine::GameObject*)Engine::Globals::gScene->pGetVectorOfEntities()->at(i))->ApplyPhysicsToEntity();
                }
                //���������� ��������� ��������//
                Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->Update();
                Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->ExecuteScript();
            }

            if (Engine::Globals::states.useSceneCamera)
            {
                //��������� ����� c �������� �������//
                if (Engine::Globals::gScene->pGetActiveCamera() != nullptr)
                {
                    Engine::Globals::gScene->UpdateActiveCamera();

                    std::thread drawWithActiveCamera(
                        &Engine::Renderer::DrawScene,
                        &Engine::renderer,
                        ImguiDrawData,
                        Engine::Globals::gScene,
                        *Engine::Globals::gScene->pGetActiveCamera()
                    );

                    drawWithActiveCamera.join();
                }
                else {
                    sceneEditor.editorCamera.Update();

                    std::thread drawWithEditorCamera(
                        &Engine::Renderer::DrawScene,
                        &Engine::renderer,
                        ImguiDrawData,
                        Engine::Globals::gScene,
                        sceneEditor.editorCamera
                    );

                    drawWithEditorCamera.join();
                }
            }
            else if (Engine::Globals::gScene->pGetActiveCamera() == nullptr) {
                //���� �������� ������ �� ����������//
                //��� ���� ������������� �������� ������� �� ���������//
                //�� ��������� �� ������ ���������//

                sceneEditor.editorCamera.Update();

                std::thread drawWithEditorCamera(
                    &Engine::Renderer::DrawScene,
                    &Engine::renderer,
                    ImguiDrawData,
                    Engine::Globals::gScene,
                    sceneEditor.editorCamera
                );

                drawWithEditorCamera.join();
            }
        }
    }
}

//������������ ��������//
void Application::Clear() {
    //�������� ����������� GPU//

    vkQueueWaitIdle(Engine::renderer.device.GetGraphicsQueue());
    vkDeviceWaitIdle(Engine::renderer.device.Get());

    //������������ �������� ������� �����//
    Engine::Globals::gScene->CleanScene();
    //������������ �������� �����//
    delete Engine::Globals::gScene;

    if (ENABLE_IMGUI) {
        ImGui::DestroyContext();
        ImGui_ImplWin32_Shutdown();
        ImGui_ImplVulkan_Shutdown();
    }

    DestroyWindow(hwnd);

    Engine::renderer.clear();
    lua_close(Engine::Lua::gLuaState);
}

uint32_t Application::GetWidth() {
    return WindowWidth;
}

uint32_t Application::GetHeight() {
    return WindowHeight;
}