#pragma once
#include	"MainVulkan.h"
#include	<iostream>
#include	<Windows.h>



//��������� ������������ 
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

class EditorMenu{
	bool enableEditor = true;
	bool show_demo_window = true;
	bool startButton;
	bool resetPhysicsIfSceneStopped = true;
	bool closeWindow = false;
	int selected = -1;
public:
	void menuLoop(std::vector<Lava::GameObject> &GameObjects, bool &sceneIsPlaying, GLFWwindow* window){
		if (enableEditor) {
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::ShowDemoWindow(&show_demo_window);

			//MenuBar ��������� �����
			ImGui::BeginMainMenuBar();

			//Todo: ����� ���� "����" ����� ������������ �������� � ���������� ����
			if (ImGui::BeginMenu(u8"����")) {
				ImGui::MenuItem(u8"�����", "ALT + F4", &closeWindow);
				ImGui::EndMenu();
				if (closeWindow) {
					glfwSetWindowShouldClose(window, GLFW_TRUE);
				}
			}

			// Todo: ��� ������� ����� ��������������� ������ ��������� ������,
			//� � ���������� ���������� ��������
			if (ImGui::BeginMenu(u8"������ �����")) {
				ImGui::MenuItem(u8"�����", "", &startButton);
				if (startButton) {
					sceneIsPlaying = true;
					resetPhysicsIfSceneStopped = true;
				}
				else {
					sceneIsPlaying = false;
					if (resetPhysicsIfSceneStopped) {
						for (size_t i = 0; i < GameObjects.size(); i++) {
							//����� ������������� ����, ���� ����� �� ��������
							GameObjects[i].getMesh()->transform.setInitialTransform();
							GameObjects[i].getRigidBody()->setRigidBodyTransformFromMesh(*GameObjects[i].getMesh());
							if (GameObjects[i].getRigidBody()->getBulletRigidBody() != nullptr) {
								GameObjects[i].getRigidBody()->getBulletRigidBody()->clearForces();
								GameObjects[i].getRigidBody()->getBulletRigidBody()->clearGravity();
							}
							
						}
						resetPhysicsIfSceneStopped = false;
						Lava::solver->reset();
					}
				}
				ImGui::EndMenu();
			}

			//TODO: � ���� ������ ����� ������������� ������� ��� ��������� 
			if (ImGui::BeginMenu(u8"���")) {
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();


			//Todo: �������� ������� ��������
			ImGui::Begin(u8"Hierarchy");
			
			for (size_t i = 0; i < GameObjects.size(); i++) {
				bool pressed = ImGui::Button(GameObjects[i].getName().c_str());
				if(pressed){
					spdlog::info("Object with number {:08d} is selected" ,i);
					selected = i;
				}
			/*	if (ImGui::TreeNode(GameObjects[i].getName().c_str())) {
					ImGui::TreePop();
				}*/
				//ImGui::End();
			}
			if(ImGui::Button("+ Add Object")){
			
			}

			if (selected != -1) {
				ImGui::Begin("Properties");
				ImGui::End();
			}

			ImGui::End();
		}
		else{
			sceneIsPlaying = true;
		}
	}
	bool isEditorEnabled(){
		return enableEditor; 
	}
}editorMenu;


class Application {
	GLFWwindow* window;

	uint32_t width;
	uint32_t height;

	double time,deltaTime,lastFrameTime;
	bool sceneIsPlaying = false;

	//--------------------------------------------------------------
public:
	//��������� ����������� GLFW, Vulkan, ImGUI 
	void Init(){
		//��������� ���������� GLFW
		if (glfwInit() != GLFW_TRUE) {
			throw std::runtime_error("Failed to initialize GLFW");
		}
		width = Lava::WIDTH;
		height = Lava::HEIGHT;

		glfwWindowHint(GLFW_CLIENT_API, false);
		glfwWindowHint(GLFW_RESIZABLE,false);
		
		window = glfwCreateWindow(width, height, "Lava Engine", nullptr, nullptr);
		
		glfwSetCursorPosCallback(window,mouseCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		
		// ��������� ������������ �������� ������ Vulkan
		Lava::init(window);

		
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup Dear ImGui style
		//ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		ImGui::StyleColorsClassic();

		// Setup Platform/Renderer bindings
		if (ImGui_ImplGlfw_InitForVulkan(window, true) != true) {
			throw std::runtime_error("Failed to init imguiglfw for vulkan");
		}

		// �������� � ImGUI �������� �������� ������ Vulkan
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
		ImGui_ImplVulkan_Init(&init_info,Lava::renderPass.getRenderPass());

		// �������� ������� ��� ImGUI � ������ ����������
		Lava::CommandBuffer oneTimeSubmitCommandBuffer;
		oneTimeSubmitCommandBuffer.beginCommandBuffer(Lava::device.get(), Lava::commandPool.get());
		ImGui_ImplVulkan_CreateFontsTexture(oneTimeSubmitCommandBuffer.get());
		oneTimeSubmitCommandBuffer.endCommandBuffer();
		oneTimeSubmitCommandBuffer.submitCommandBuffer(Lava::device.getGraphicsQueue());
		oneTimeSubmitCommandBuffer.freeCommandBuffer(Lava::device.get(), Lava::commandPool.get());

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		
		time = 0;
		lastFrameTime = 0;
		deltaTime = 0;
	}
	//	������� ����
	void Loop() {
		while (!glfwWindowShouldClose(window)) {
			// ���������� �������, ������������ �� ��������� ������ ����� (������ �������)
			time = glfwGetTime();
			deltaTime =	time - lastFrameTime;
			lastFrameTime = time;

			// ����������� ������ � ������� WASD + LShift
			Lava::camera.processInput(window, deltaTime);

			// ������ � ���� ImGUI ���� ����������� ��������� ���������� ��������� �����
			editorMenu.menuLoop(Lava::gameObjects,sceneIsPlaying,window);

			if (sceneIsPlaying) {
				for (size_t i = 0; i < Lava::gameObjects.size(); i++) {
					// �������� ������������� 
					Lava::gameObjects[i].syncMeshTransformWithRigidBody();
				}
				Lava::dynamicsWorld->stepSimulation(deltaTime);
			}

			//	��� ������� ������� LAlt ��������� ����� ����� 
			if (glfwGetKey(window,GLFW_KEY_LEFT_ALT)==GLFW_PRESS) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}else{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			//	���������� ������ ��� ������ ��������� ImGUI
			if (editorMenu.isEditorEnabled()) {
				ImGui::Render();
			}

			
			ImDrawData* draw_data = ImGui::GetDrawData();

			//	������������� ����������� �������� �� �����
			Lava::DrawObjects(
				window,
				draw_data,
				editorMenu.isEditorEnabled()
			);
			
			//	��������� ������� ����
			glfwPollEvents();
		}
	}

	//	������������ ������
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

double lastX = Lava::WIDTH / 2;
double lastY = Lava::HEIGHT / 2;

//	���������� �������� ������� (��� ������� ������� Alt)
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

//	���������� ������� ������ ���� (�������� ��� �� �������)
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
	double x,y;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		glfwGetCursorPos(window,&x,&y);
		glm::vec3 ray = Lava::castRay(x,y);
		spdlog::info("Ray X: {:1.5f} Y: {:1.5f} Z: {:1.5f}",ray.x,ray.y,ray.z);

	}
}

Application app;