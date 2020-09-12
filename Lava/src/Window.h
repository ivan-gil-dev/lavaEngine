#pragma once
#include "MainVulkan.h"
#include <iostream>
#include <Windows.h>
using namespace Lava;


double lastX = Lava::WIDTH / 2;
double lastY = Lava::HEIGHT / 2;

//	обработка движений курсора 
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
//	обработка нажатий кнопок мыши
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
class Menu{



};

class Window {
	GLFWwindow* window;
	uint32_t width;
	uint32_t height;

	//	переменные для вычисления времени отрисовки одного кадра
	double time,deltaTime,lastFrameTime;

	//	показать демо ImGui
	bool show_demo_window;

	//	воспроизвидение сцены (начало симуляции физики)
	bool sceneIsPlaying = false;

	//	кнопка воспроизвидения сцены
	bool startButton;

	//	вернуть значения трансформации всех объектов к исходным
	bool resetPhysicsIfSceneStopped = true;

	//	закрыто ли окно
	bool closeWindow = false;

	//--------------------------------------------------------------
public:
	//	инициализация компонентов движка перед началом отрисовки
	void Init(){
		//	инициализация GLFW
		if (glfwInit() != GLFW_TRUE) {
			throw std::runtime_error("Failed to initialize GLFW");
		}
		width = Lava::WIDTH;
		height = Lava::HEIGHT;

		//	убираем контекст OpenGL
		glfwWindowHint(GLFW_CLIENT_API, false);

		glfwWindowHint(GLFW_RESIZABLE,false);
		
		window = glfwCreateWindow(width, height, "Lava Engine", nullptr, nullptr);
		

		//	обработка движений курсора 
		glfwSetCursorPosCallback(window,mouseCallback);

		//	обработка нажатий кнопок мыши
		glfwSetMouseButtonCallback(window, mouseButtonCallback);

		//	инициализация компонентов Vulkan
		Lava::init(window);

		// настройка ImGUI
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// Setup Platform/Renderer bindings
		if (ImGui_ImplGlfw_InitForVulkan(window, true) != true) {
			throw std::runtime_error("Failed to init imguiglfw for vulkan");
		}
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = Lava::instance.get();
		init_info.PhysicalDevice = Lava::physicalDevice.get();
		init_info.Device = *Lava::device.pGet();
		init_info.QueueFamily = Lava::physicalDevice.getQueueIndices().graphicsQueueIndex;
		init_info.Queue = Lava::device.getGraphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = Lava::descriptorPoolForImgui.get();
		init_info.Allocator = nullptr;
		init_info.MinImageCount = Lava::swapchain.getInfo().minImageCount;
		init_info.ImageCount = Lava::swapchain.getInfo().minImageCount;
		init_info.CheckVkResultFn = imguiErrFunction;
		ImGui_ImplVulkan_Init(&init_info,renderPass.getRenderPass());

		Lava::CommandBuffer oneTimeSubmitCommandBuffer;
		oneTimeSubmitCommandBuffer.beginCommandBuffer(Lava::device.get(), Lava::commandPool.get());
		ImGui_ImplVulkan_CreateFontsTexture(oneTimeSubmitCommandBuffer.get());
		oneTimeSubmitCommandBuffer.endSubmitAndFreeCommandBuffer(Lava::device.get(), Lava::commandPool.get(),Lava::device.getGraphicsQueue());
		ImGui_ImplVulkan_DestroyFontUploadObjects();
		show_demo_window = false;

		time = 0;
		lastFrameTime = 0;
		deltaTime = 0;
	}
	//	главный цикл
	void Loop() {
		while (!glfwWindowShouldClose(window)) {
			time = glfwGetTime();
			deltaTime =	time - lastFrameTime;
			lastFrameTime = time;

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			
			ImGui::ShowDemoWindow(&show_demo_window);

			//	обработка нажатий клавиш для передвижения камеры
			camera.processInput(window , deltaTime);

			//	меню ImGui
			ImGui::BeginMainMenuBar();
			if(ImGui::BeginMenu(u8"Файл")){
				ImGui::MenuItem(u8"Выход","ALT + F4",&closeWindow);
				ImGui::EndMenu();
				if (closeWindow) {
					break;
				}
			}

			if (ImGui::BeginMenu(u8"Правка")) {

				ImGui::EndMenu();
			}
			

			if (ImGui::BeginMenu(u8"Начало сцены")) {
				ImGui::MenuItem(u8"Старт","",&startButton);
				//	обработка нажатия кнопки старт
				if (startButton) {
					sceneIsPlaying = true;
					resetPhysicsIfSceneStopped = true;
					
				}
				//	если кнопка не нажата, то сбрасывация трансформация объектов
				else {
					sceneIsPlaying = false;

					if (resetPhysicsIfSceneStopped) {
						for (size_t i = 0; i < gameObjects.size(); i++) {
							gameObjects[i].setInitialTransform();
							gameObjects[i].setRigidBodyObjectTransform();
							if (gameObjects[i].getRigidBody() != nullptr) {
								gameObjects[i].getRigidBody()->clearForces();
								gameObjects[i].getRigidBody()->clearGravity();
							}
						}
						resetPhysicsIfSceneStopped = false;
						solver->reset();
					}
					
					
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(u8"Вид")) {

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		
		

			ImGui::Begin(u8"Test");
			for (size_t i = 0; i < gameObjects.size(); i++) {
				if (ImGui::TreeNode(gameObjects[i].getName().c_str())) {
					ImGui::TreePop();
				}
				
				//ImGui::End();
			}
				
			
			ImGui::End();
			//	синхронизация физического движка с графическим
			if (sceneIsPlaying) {
				for (size_t i = 0; i < gameObjects.size(); i++) {
					gameObjects[i].syncRigidBodyWithGraphics();
				}
				dynamicsWorld->stepSimulation(deltaTime);
			}

			//	если нажата клавиша ALT то курсор исчезает
			if (glfwGetKey(window,GLFW_KEY_LEFT_ALT)==GLFW_PRESS) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}else{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			//	отрисовка элементов ImGui
			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();

			//	отрисовка объектов движка
			Lava::DrawObjects(
				window,
				draw_data
			);
			
			//	обработка событий GLFW 
			glfwPollEvents();
		}
		
	}
	//	освобождение памяти
	void Clear() {
		Lava::clear();
		glfwTerminate();
	}

	GLFWwindow* Get() {
		return window;
	}
	uint32_t GetWidth() {
		return width;
	}
	uint32_t GetHeight() {
		return height;
	}

};

//	обработка движений курсора 
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
		if (Lava::camera.firstMouse) {
			lastX = xpos;
			lastY = ypos;
			Lava::camera.firstMouse = false;
		}

		double xoffset = xpos - lastX;
		double yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		Lava::camera.cursorUpdate(xoffset,yoffset);
	}else{
		Lava::camera.firstMouse = true;
	}
}

//	обработка нажатий кнопок мыши
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
	double x,y;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		glfwGetCursorPos(window,&x,&y);
		glm::vec3 ray = castRay(x,y);
		spdlog::info("Ray X: {:1.5f} Y: {:1.5f} Z: {:1.5f}",ray.x,ray.y,ray.z);

	}
}

Window window;