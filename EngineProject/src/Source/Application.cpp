#include "../Headers/Application.h"
#include <shobjidl.h>
#include <algorithm>
#include "../Headers/Renderer/Renderer.h"
#include <Shlwapi.h>
#include <thread>
#include "../Headers/Script.h"

/*Определение функции для вызова из другого проекта*/
typedef void (*DemoFunc)(
    Engine::Scene*,
    btDynamicsWorld*
    );
DemoFunc demo;

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

/*Диалог для открытия файла json*/
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
                        //                      Получение пути к файлу
                        //                      и приведение типов из PWSTR -> std::string
                        std::wstring str(pszFilePath);
                        std::string path(str.begin(), str.end());

                        TCHAR buffer[MAX_PATH] = { 0 };
                        GetCurrentDirectory(MAX_PATH, buffer);

                        /*Путь к выполняемому в данный момент EXE файлу*/
                        std::wstring buffer2 = buffer;
                        std::string currentDir(buffer2.begin(), buffer2.end());

                        /*Получение пути к сцене относительно EXE файла*/
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

/*Диалог для сохранения файла json*/
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

/*Начальная настройка редактора*/
void SceneEditor::InitEditor(HWND hwnd) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    /*Визуальные настройки панелей */
    ImGui::StyleColorsClassic();
    ImGuiStyle& style = ImGui::GetStyle(); {
        style.WindowRounding = 0.0f;
        style.WindowPadding = ImVec2(2, 2);
        style.FramePadding = ImVec2(2, 2);
        style.WindowTitleAlign = ImVec2(0.5, 0.5);
        style.WindowMenuButtonPosition = ImGuiDir_Right;
    }

    /*Привязка компонентов ImGui к текущему окну*/
    if (ImGui_ImplWin32_Init(hwnd) != true) {
        std::string error = "Failed to init imgui for win32";
        throw std::runtime_error(error);
    }

    /*Передача активных компонентов Vulkan в ImGui*/
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

    if (!ImGui_ImplVulkan_Init(&init_info, Engine::renderer.renderPass.GetRenderPass())) {
        std::string error = "Failed to init imgui for Vulkan";
        throw std::runtime_error(error);
    }

    Engine::CommandBuffer oneTimeSubmitCommandBuffer;

    //Использование буфера команд для загрузки шрифтов в GPU
    //Затем Imgui будет использовать загруженный шрифт
    oneTimeSubmitCommandBuffer.AllocateCommandBuffer(Engine::renderer.device.Get(), Engine::renderer.commandPool.Get());
    oneTimeSubmitCommandBuffer.BeginCommandBuffer();

    /*Создание текстуры из шрифта(преобразование шрифта в изображение)*/
    if (!ImGui_ImplVulkan_CreateFontsTexture(oneTimeSubmitCommandBuffer.Get())) {
        std::string error = "Failed to load fonts to Imgui";
        throw std::runtime_error(error);
    }

    oneTimeSubmitCommandBuffer.EndCommandBuffer();
    oneTimeSubmitCommandBuffer.SubmitCommandBuffer(Engine::renderer.device.GetGraphicsQueue());
    oneTimeSubmitCommandBuffer.FreeCommandBuffer(Engine::renderer.device.Get(),
        Engine::renderer.commandPool.Get());

    /*	освободить ресурсы для загрузки шрифта*/
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

/*Поведение панелей ImGui и порядок отрисовки*/
void SceneEditor::DrawEditor(HWND hwnd, std::vector<Engine::Entity*>& Entities) {
    if (enableEditor) {
        ImGui::ShowDemoWindow(&DemoWindowActive);

        /*Начало записи настроек меню*/
        ImGui::BeginMainMenuBar();

        /*получение высоты меню*/
        MenubarHeight = ImGui::GetWindowHeight();

        /*Сдвинуть вьюпорт сцены на высоту менюбара*/
        Engine::renderer.rendererViewport.y = MenubarHeight;

        /*Уменьшить размер вьюпорта сцены на высоту менюбара*/
        Engine::renderer.rendererViewport.height = Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight;

        /*Начало записи настроек меню File*/
        if (ImGui::BeginMenu(u8"File")) {
            /*Начало записи настроек пункта меню New*/
            if (ImGui::MenuItem("New"))
            {
                /*Сброс выбранного объекта*/
                SelectedItem_ID = -1;

                //Ожидание бездействия очереди
                //для выполнения команд отрисовки
                Engine::renderer.WaitForDrawFences();

                /* Создание потока для запуска метода New*/
                std::thread thr(
                    &Engine::Scene::New_FromThread,
                    Engine::Globals::gScene,
                    std::ref(LoadingIsEnded)
                );

                /*Сброс флага окончания загрузки*/
                LoadingIsEnded = false;

                /*Запуск потока*/
                thr.detach();

                MSG msg = { };

                //До того, как фоновый поток выставит флаг LoadingIsEnded,
                //окно будет получать сообщения
                while (!LoadingIsEnded)
                {
                    if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                /*сброс векторов камеры редактора*/
                editorCamera.Reset();

                /*    Выставить флаг для пересоздания буферов команд*/
                Engine::renderer.SetRebuildTrigger();
            }

            /*Начало записи настроек пункта меню Open*/
            ImGui::MenuItem(u8"Open", "", &OpenFileDialog);
            if (OpenFileDialog) {
                //Никакой объект не выбран
                SelectedItem_ID = -1;

                //Обзор (Путь к сцене в JSON формате)
                std::string path = WinApiOpenDialog();

                //Если путь к файлу не пустой
                if (path != "")
                {
                    spdlog::info("Loading...");
                    std::cout << path << std::endl;

                    //Ожидание освобождения очереди
                    Engine::renderer.WaitForDrawFences();

                    //Поток загрузки сцены
                    std::thread thr(
                        //Ссыдка на метод загрузки сцены
                        &Engine::Scene::Load_FromThread,
                        //Указатель на сцену
                        Engine::Globals::gScene,
                        //Путь к сцене
                        path,
                        //Ссылка на atomic<bool>
                        std::ref(LoadingIsEnded)
                    );

                    //Сброс поля LoadingIsEnded//
                    LoadingIsEnded = false;

                    //Запуск фонового потока//
                    thr.detach();

                    MSG msg = { };
                    //Пока сцена загружается окно будет получать сообщения//
                    while (!LoadingIsEnded)
                    {
                        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }

                    //Сброс камеры сцены//
                    editorCamera.Reset();

                    spdlog::info("Done!");
                }

                //Пересоздать swapchain//
                Engine::renderer.recreateSwapchain();

                //Перестроить буферы команд вывода сцены//
                Engine::renderer.SetRebuildTrigger();

                //Диалог закрыт...
                OpenFileDialog = false;
            }

            bool Save = false;
            bool SaveAs = false;

            //Пункт меню "Save"
            ImGui::MenuItem("Save", "", &Save);
            if (Save) {
                //Если путь к сцене не пустой
                if (Engine::Globals::gScene->GetScenePath() != "")
                {
                    spdlog::info("Saving...");

                    //Получение пути к текущей сцене
                    std::cout << Engine::Globals::gScene->GetScenePath() << std::endl;

                    std::thread thr(
                        //Ссылка на метод сохранения
                        &Engine::Scene::Save_FromThread,
                        //Указатель на сцену
                        Engine::Globals::gScene,
                        //Ссылка на atomic<bool>
                        std::ref(LoadingIsEnded)
                    );

                    thr.join();

                    spdlog::info("Done!");
                }
                else {
                    SaveAs = true;
                }
            }

            //Пункт меню "SaveAs"...
            ImGui::MenuItem("Save As", "", &SaveAs);
            if (SaveAs) {
                //Обзор (Сохранение файла)
                std::string path = WinApiSaveDialog();
                if (path != "")
                {
                    spdlog::info("Saving...");
                    std::cout << path << std::endl;

                    //Поток сохранения сцены
                    std::thread thr(
                        //Ссылка на метод
                        &Engine::Scene::SaveAs_FromThread,
                        //Указатель на сцену
                        Engine::Globals::gScene,
                        //Путь к сцене
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
        }    //Конец меню//

        if (ImGui::BeginMenu(u8"Simulation")) {
            ImGui::MenuItem(u8"Start", "", &StartButtonActive);

            if (StartButtonActive) {//Обработка нажатия кнопки "старт"//
                Engine::Globals::gIsScenePlaying = true; //Проигрывание сцены//
                ResetPhysics = true; //Физика будет сброшена после остановки симуляции//
            }
            else {
                Engine::Globals::gIsScenePlaying = false;

                //Сброс параметров объектов//
                if (ResetPhysics) {
                    for (size_t i = 0; i < Entities.size(); i++) {
                        //Сброс позиции сущности//
                        Entities.at(i)->Transform.ResetTransform();

                        if (Entities.at(i)->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT) {
                            Engine::GameObject* obj = (Engine::GameObject*)Entities.at(i);

                            if (obj->pGetComponent<Engine::RigidBody*>() != nullptr) {
                                //Применение параметров перемещениия к физическому телу//
                                obj->ApplyEntityTransformToRigidbody();

                                btRigidBody* pRigidbody = obj->pGetComponent<Engine::RigidBody*>()->GetBulletRigidBody();

                                //Обнуление физических параметров//
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
            ImGui::EndMenu();//Конец меню//
        }

        if (ImGui::BeginMenu(u8"View")) { //Меню: "Вид"//
            ImGui::MenuItem(u8"Hierarchy Panel", "", &ShowHierarchyPanel);//Показать панель иерархии//
            ImGui::MenuItem(u8"Properties Panel", "", &ShowPropertiesPanel);//Показать панель свойств//
            ImGui::MenuItem(u8"Skybox", "", &Engine::Globals::states.showSkybox);//Показать скайбокс//
            ImGui::MenuItem(u8"Mesh", "", &Engine::Globals::states.showMeshes);//Показать 3D модель//
            ImGui::MenuItem(u8"Rigidbody Mesh", "", &Engine::Globals::states.showRigidbodyMeshes);//Показать контур физического объекта//
            ImGui::MenuItem(u8"Draw Shadows", "", &Engine::Globals::states.drawShadows);//Рисовать тени//
            ImGui::MenuItem(u8"Use Scene Camera", "", &Engine::Globals::states.useSceneCamera);//Использовать активную камеру сцены//
            ImGui::EndMenu(); //Конец меню//
        }
        ImGui::EndMainMenuBar(); //Конец менюбара//

        //Панель иерархии//
        if (ShowHierarchyPanel) {
            bool opened = true;
            //Начало записи параметров панели иерархии//
            ImGui::Begin(u8"Hierarchy", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            //Сдвинуть вьюпорт вправо на ширину панели иерархии//
            Engine::renderer.rendererViewport.x = ImGui::GetWindowWidth();

            //Ширина вьюпорта умешилась на ширину пане иерархии//
            Engine::renderer.rendererViewport.width = Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowWidth();

            //Учесть высоту менюбара при выводе панели иерархии//
            ImGui::SetWindowPos(ImVec2(0, MenubarHeight));
            ImGui::SetWindowSize(ImVec2(ImGui::GetWindowWidth(),
                Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
            );

            //Список объектов//
            for (size_t i = 0; i < Entities.size(); i++) {
                //Пункт списка//
                if (ImGui::TreeNode(Entities.at(i)->GetName().c_str())) {
                    //При нажатии на строку присваивается ID объекта этой ветке//
                    ImGui::PushID(Entities.at(i)->GetID());
                    ImGui::PopID();
                    ImGui::TreePop();
                }

                //Обработка нажатия на пункт списка//
                if (ImGui::IsItemClicked()) {
                    //Выбран i-й объект//
                    SelectedItem_ID = (int)i;
                    spdlog::info("Object with ID {:08d} is selected", Entities.at(i)->GetID());

                    //Показать панель свойств//
                    ShowPropertiesPanel = true;
                }

                //Обработка нажатия правой кнопки мыши на пункт списка//
                if (ImGui::IsItemClicked(1))
                {
                    SelectedItem_ID = (int)i;
                    //Вызов всплывающего окна Popup//
                    ImGui::OpenPopup("Popup");
                }
            }

            if (ImGui::BeginPopup("Popup")) {
                //Создание дупликата
                if (ImGui::MenuItem("Duplicate")) {
                    if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT)
                    {
                        Engine::GameObject* obj = new Engine::GameObject;
                        obj->SetName(Entities[SelectedItem_ID]->GetName());
                        obj->SetID(int(obj));
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

                            //Запуск фонового потока//
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
                //кнопка Delete//
                if (ImGui::MenuItem("Delete")) {
                    //Ожидание освобождения очереди для отрисовки//
                    Engine::renderer.WaitForDrawFences();

                    //Удаление происходит по разному для каждого типа объекта//
                    if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT)
                    {
                        //Освобождение памяти//
                        delete Entities[SelectedItem_ID];
                        //Удаление из вектора//
                        Entities.erase(Entities.begin() + SelectedItem_ID);
                    }
                    else
                        if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_DIRECTIONAL_LIGHT_OBJECT)
                        {
                            /*Поиск аттрибутов i-го источника света в векторе атрибутов*/
                            for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->size(); i++)
                            {
                                /*Если аттрибуты найдены, то они удаляются из вектора атрибутов*/
                                if (Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->at(i)
                                    == ((Engine::DirectionalLightObject*)Entities[SelectedItem_ID])->pGetDirectionalLightUniformData())
                                {
                                    Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->erase(
                                        Engine::Globals::gScene->pGetVectorOfDirectionalLightAttributes()->begin() + i
                                    );
                                }
                            }
                            //Освобождение памяти//
                            delete Entities[SelectedItem_ID];
                            //Удаление из вектора сущностей//
                            Entities.erase(Entities.begin() + SelectedItem_ID);
                        }
                        else if (Entities[SelectedItem_ID]->GetEntityType() == Engine::ENTITY_TYPE_POINTLIGHT_OBJECT)
                        {
                            /*Поиск аттрибутов i-го источника света в векторе атрибутов*/
                            for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->size(); i++)
                            {
                                //Если аттрибуты найдены, то они удаляются из вектора атрибутов//
                                if (Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->at(i)
                                    == ((Engine::PointLightObject*)Entities[SelectedItem_ID])->pGetPointLightUniformData())
                                {
                                    Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->erase(
                                        Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->begin() + i
                                    );
                                }
                            }
                            //Освобождение памяти//
                            delete Entities[SelectedItem_ID];
                            //Удаление из вектора сущностей//
                            Entities.erase(Entities.begin() + SelectedItem_ID);
                        }
                        else {
                        }
                    //Текущий объект не выбран//
                    SelectedItem_ID = -1;
                }

                ImGui::EndPopup();
            }

            //Добавление объекта//
            if (ImGui::Button(u8"Add+")) {
                //Вызов всплывающего окна "Select Object Type"//
                ImGui::OpenPopup("Select Object Type");
            }

            if (ImGui::BeginPopupModal("Select Object Type"))
            {
                ImGui::Text("Object types");
                ImGui::Separator();
                //Кнопка, при нажатии на которую создается игровой объект//
                if (ImGui::MenuItem("Game Object"))
                {
                    Engine::GameObject* obj = new Engine::GameObject;
                    //obj->AddComponent<Engine::Mesh>();
                    //obj->pGetComponent<Engine::Mesh*>()->CreateMesh("");
                    obj->SetID((int)obj);
                    Entities.push_back(obj);
                }

                //Кнопка, при нажатии на которую создается точечный источник света//
                if (ImGui::MenuItem("Point Light"))
                {
                    Engine::PointLightObject* obj = new Engine::PointLightObject;

                    obj->SetID((int)obj);
                    Engine::Globals::gScene->pGetVectorOfSpotlightAttributes()->push_back(obj->pGetPointLightUniformData());
                    Entities.push_back(obj);
                }

                //Кнопка, при нажатии на которую создается направленный источник света//
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
            //растяжение 3D вьюпорта если панель иерархии не отображена//
            Engine::renderer.rendererViewport.x = 0;
            Engine::renderer.rendererViewport.width = static_cast<float>(Engine::renderer.swapchain.GetInfo().imageExtent.width);
        }//конец панели иерархии//

        //Панель свойств//
        if (ShowPropertiesPanel) {
            bool opened = true;

            //Начало записи параметров панели свойств//
            ImGui::Begin(u8"Properties", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x, MenubarHeight));
            ImGui::SetWindowSize(
                ImVec2(Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x,
                    Engine::renderer.swapchain.GetInfo().imageExtent.height - MenubarHeight)
            );

            //Расположение 3D вьюпорта слева от панели свойств//
            Engine::renderer.rendererViewport.width -= Engine::renderer.swapchain.GetInfo().imageExtent.width - ImGui::GetWindowPos().x;

            //Если текущий объект выбран//
            if (SelectedItem_ID >= 0) {
                //минимальное значение типа данных float//
                float min = -FLT_MAX;
                //максимальное значение типа данных float//
                float max = FLT_MAX;

                //Вывод свойств в зависимости от типа объекта//
                switch (Entities.at(SelectedItem_ID)->GetEntityType()) {
                    //Вывод свойств для игрового объекта//
                case Engine::ENTITY_TYPE_GAME_OBJECT:
                {
                    //Получение свойств объектов//
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

                    //Свойства перемещения объектов//
                    if (ImGui::CollapsingHeader(u8"Translation")) {
                        ImGui::DragFloat3("Position", (float*)&objectPos, 1.0f, min, max);
                        ImGui::DragFloat3("Rotation", (float*)&objectRotation, 1.0f, min, max);
                        ImGui::DragFloat3("Scale", (float*)&objectScale, 1.0f, min, max);
                    }

                    //Свойства физического тела//
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

                    //Свойства трехмерной модели//
                    if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>() != nullptr)
                    {
                        //Получение материала модели//
                        mat = ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->GetMaterial();
                        if (ImGui::CollapsingHeader(u8"Mesh")) {
                            //Изменение оптических свойств//
                            ImGui::DragFloat("Shininess", (float*)&mat.shininess, 0.1f, min, max);
                            ImGui::DragFloat("Metallic", (float*)&mat.metallic, 0.1f, min, max);
                            ImGui::DragFloat("Roughness", (float*)&mat.roughness, 0.1f, min, max);
                            ImGui::DragFloat("Occlusion", (float*)&mat.ao, 0.1f, min, max);

                            //Вывод пути к трехмерному объекту//
                            if (((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->pGetMeshPath() != "")
                            {
                                ImGui::TextWrapped(((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->pGetMeshPath().c_str());
                            }

                            //Обзор (загрузка меша)//
                            if (ImGui::Button("Browse Mesh"))
                            {
                                //Получить путь к трехмерному объекту через диалоговое окно//
                                std::string path = WinApiOpenDialog();
                                if (path != "")
                                {
                                    std::cout << path << std::endl;
                                    std::replace(path.begin(), path.end(), '\\', '/');

                                    Engine::renderer.WaitForDrawFences();

                                    //Удаление стандартной трехмерной модели (куба)
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->DeleteComponent<Engine::Mesh>();

                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::Mesh>();

                                    //Загрузка модели по выбранному пути//
                                    std::thread thr(
                                        //Ссылка на метод загрузки модели//
                                        &Engine::Mesh::CreateMesh_FromThread,
                                        //Указатель на трехмерный объект//
                                        ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>(),
                                        //Путь к объекту//
                                        path,
                                        //Ссылка на atomic<bool>//
                                        std::ref(LoadingIsEnded)
                                    );

                                    LoadingIsEnded = false;

                                    //Запуск фонового потока//
                                    thr.detach();

                                    MSG msg = { };
                                    //Пока фоновый поток будет выполняться//
                                    //Окно будет получать сообщения//
                                    while (!LoadingIsEnded)
                                    {
                                        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                                            TranslateMessage(&msg);
                                            DispatchMessage(&msg);
                                        }
                                    }

                                    //Пересоздать swapchain//
                                    Engine::renderer.recreateSwapchain();
                                    //Перестроить буферы//
                                    Engine::renderer.SetRebuildTrigger();
                                }
                            }

                            if (!((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>()->IsMaterialsFound())
                            {
                                ImGui::BulletText("MTL File Not Found!");
                            }
                        }
                    }

                    //Если сцена не проигрывается, то применить новые свойства//
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
                        //Кнопка создания трехмерной модели, если сущность не содержит модель//
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
                        //Выбор формы физического тела -> создание физического тела
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
                                if (ImGui::MenuItem("Mesh"))
                                {
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->AddComponent<Engine::RigidBody>();
                                    ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::RigidBody*>()->CreateRigidBody(
                                        ((Engine::GameObject*)Entities.at(SelectedItem_ID))->pGetComponent<Engine::Mesh*>(),
                                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                                        SelectedItem_ID
                                    );
                                }

                                Engine::renderer.SetRebuildTrigger();
                                ImGui::EndPopup();
                            }
                        }
                    }
                }

                break;

                //Панель свойств для точечного источника света//
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

                    //Изменение атрибутов источника света//
                    if (ImGui::CollapsingHeader(u8"Light Params")) {
                        ImGui::DragFloat3("Color", (float*)&pointLightAttributes.lightColor, 1.0f, min, max);

                        ImGui::DragFloat("Ambient", &pointLightAttributes.ambient);
                        ImGui::DragFloat("Diffuse", &pointLightAttributes.diffuse);
                        ImGui::DragFloat("Specular", &pointLightAttributes.specular);

                        ImGui::DragFloat("Constant", &pointLightAttributes.constant);
                        ImGui::DragFloat("Linear", &pointLightAttributes.linear);
                        ImGui::DragFloat("Quadrantic", &pointLightAttributes.quadrantic);
                    }

                    //Если сцена не проигрывается, то применить новые свойства//
                    if (!Engine::Globals::gIsScenePlaying) {
                        Entities.at(SelectedItem_ID)->SetName(name);
                        Entities.at(SelectedItem_ID)->Transform.SetTranslation(objectPos);
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

                    //Изменение атрибутов источника света//
                    ImGui::DragFloat3("Direction", (float*)&dirLightAttributes.lightDirection, 0.1f, min, max, "%.3f", 0.1f);
                    if (ImGui::CollapsingHeader(u8"Light Params")) {
                        ImGui::DragFloat3("Color", (float*)&dirLightAttributes.lightColor, 0.1f, 0.0f, 255.f, "%.3f", 0.1f);
                        ImGui::DragFloat("Ambient", &dirLightAttributes.ambient);
                        ImGui::DragFloat("Diffuse", &dirLightAttributes.diffuse);
                        ImGui::DragFloat("Specular", &dirLightAttributes.specular);
                    }

                    //Если сцена не проигрывается, то применить новые свойства//
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
        //Если редактор не используется но сцена проигрывается//
        Engine::Globals::gIsScenePlaying = true;
    }
}

void RegisterClasses() {
    luabridge::getGlobalNamespace(Engine::Lua::gLuaState)
        .beginClass<Engine::Entity>("Entity")
        .addFunction("Test", &Engine::Entity::lua_Test)
        .addFunction("MoveEntity", &Engine::Entity::lua_MoveEntity)
        .addFunction("RotateEntity", &Engine::Entity::lua_RotateEntity)
        .endClass()
        .getGlobalNamespace(Engine::Lua::gLuaState);
}

//Инициализация//
void Application::Init() {
    Engine::Lua::gLuaState = luaL_newstate();
    luaL_openlibs(Engine::Lua::gLuaState);
    RegisterClasses();

    Engine::Lua::Script scr;
    scr.SetScriptPath("CoreAssets/settings.lua");
    int r = scr.doScript();

    if (r == LUA_OK)
    {
        WindowWidth = scr.getVar<lua_Integer>("windowWidth");
        WindowHeight = scr.getVar<lua_Integer>("windowHeight");
    }
    else {
        WindowWidth = 1366;
        WindowHeight = 768;
    }

    Engine::Globals::gHeight = WindowHeight;
    Engine::Globals::gWidth = WindowWidth;
    sceneEditor.enableEditor = true;
    //Заполнение оконного класса//
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
        //Создание окна в полноэкранном режиме//
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
        //Создание окна в оконном режиме//
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

    //Первоначальная настройка компонентов Vulkan//
    Engine::renderer.initVulkan(hwnd, hInstance);

    //Выделение памяти для сцены//
    Engine::Globals::gScene = new Engine::Scene;

    //Получение адреса процедуры из другого проекта//
    demo = (DemoFunc)GetProcAddress(GetModuleHandle(NULL), "DemoExe");

    //Создание сцены из другого проекта//
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

    //Time = 0;
    //LastFrameTime = 0;
    Engine::Globals::DeltaTime = 0;
}

//Главный цикл//
void Application::Execute() {
    MSG msg = { };
    while (msg.message != WM_QUIT) {
        //Обработка сообщений//
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        //Расчет времени итерации цикла//
        Time = std::chrono::high_resolution_clock::now();
        Engine::Globals::DeltaTime = (
            std::chrono::duration_cast<std::chrono::microseconds>(Time.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::microseconds>(LastFrameTime.time_since_epoch()).count()
            );
        LastFrameTime = Time;

        //Перевод в секунды
        Engine::Globals::DeltaTime /= 1000000;

        if (Engine::Globals::gIsScenePlaying) {
            //Шаг симуляции физической модели перемещения//
            float step = Engine::Globals::DeltaTime;
            Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld->stepSimulation(step, 0);
        }

        if (ENABLE_IMGUI) {
            {
                //Подготовка нового кадра ImGui//
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                //Запись данных кадра//
                sceneEditor.DrawEditor(hwnd, *Engine::Globals::gScene->pGetVectorOfEntities());

                //Подготовка данных для передачи в GPU//
                ImGui::Render();
                ImguiDrawData = ImGui::GetDrawData();
                ImGui::EndFrame();
            }
        }

        if (sceneEditor.LoadingIsEnded)
        {
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
            else {
                for (size_t i = 0; i < Engine::Globals::gScene->pGetVectorOfEntities()->size(); i++) {
                    //Применение физической модели к графической//
                    if (Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->GetEntityType() == Engine::ENTITY_TYPE_GAME_OBJECT) {
                        ((Engine::GameObject*)Engine::Globals::gScene->pGetVectorOfEntities()->at(i))->ApplyPhysicsToEntity();
                    }
                    //Обновление состояния объектов//
                    Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->Update();
                    Engine::Globals::gScene->pGetVectorOfEntities()->at(i)->ExecuteScript();
                }

                if (Engine::Globals::states.useSceneCamera)
                {
                    //Отрисовка сцены c активной камерой//
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
                }
                if (!Engine::Globals::states.useSceneCamera || Engine::Globals::gScene->pGetActiveCamera() == nullptr) {
                    //Если активная камера не определена//
                    //Или флаг использования активной камееры не выставлен//
                    //То отрисовка из камеры редактора//

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
}

//Освобождение ресурсов//
void Application::Clear() {
    //Ожидание бездействия GPU//

    vkQueueWaitIdle(Engine::renderer.device.GetGraphicsQueue());
    vkDeviceWaitIdle(Engine::renderer.device.Get());

    //Освобождение ресурсов объекта сцены//
    Engine::Globals::gScene->CleanScene();
    //Освобождение ресурсов сцены//
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