#pragma once
#define		STB_IMAGE_IMPLEMENTATION
#define		STB_IMAGE_STATIC
#define		GLFW_INCLUDE_VULKAN
#define		VK_NO_PROTOTYPES
#define		GLM_FORCE_RADIANS
#define		GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include	"../vendor/imgui.h"
#include	"../vendor/imgui_impl_glfw.h"
#include	"../vendor/imgui_impl_vulkan.h"
#include	"../vendor/glm/glm.hpp"
#include	"../vendor/glm/gtc/matrix_transform.hpp"
#include	"../vendor/glm/gtc/quaternion.hpp"
#include	"../vendor/glm/gtx/quaternion.hpp"
#include	"../vendor/volk.h"

#include	<GLFW/glfw3.h>
#include	<array>
#include	<iostream>
#include	<vector>
#include	<fstream>
#include	<chrono>
#include	<stb_image.h>

#include	"Instance.h"
#include	"PhysicalDevice.h"
#include	"DataTypes.h"
#include	"Device.h"
#include	"Buffers.h"
#include	"CommandBuffer.h"
#include	"CommandPool.h"
#include	"DescriptorPools.h"
#include	"DescriptorSetLayouts.h"
#include	"Objects.h"
#include	"Pipelines.h"
#include	"Surface.h"
#include	"Swapchain.h"
#include	"SyncObjects.h"
#include	"Images.h"



void imguiErrFunction(VkResult result) {
	if (result != VK_SUCCESS) {
		std::cout << "Imgui vulkan errcode: " << result << std::endl;
		throw std::runtime_error("Imgui fatal error");
	}
}


namespace Lava{
	static uint32_t currentFrame = 0;

	static std::vector<VkFramebuffer> createFramebuffers(
		VkDevice device,
		VkRenderPass renderPass,
		std::vector<VkImageView> swapchainImageViews, 
		VkImageView depthImageView,
		VkImageView multisamplingImageView,
		VkExtent2D swapchainExtent
	) {
		std::vector<VkFramebuffer> framebuffers;
		framebuffers.resize(swapchainImageViews.size());
		for (size_t i = 0; i < swapchainImageViews.size(); i++) {
			VkImageView attachments[] = {
				multisamplingImageView,
				depthImageView,
				swapchainImageViews[i],
			};
			VkFramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.renderPass = renderPass;
			framebufferCreateInfo.height = swapchainExtent.height;
			framebufferCreateInfo.width = swapchainExtent.width;
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.pAttachments = attachments;
			framebufferCreateInfo.attachmentCount = (uint32_t)3;
			framebufferCreateInfo.layers = 1;
			if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer");
			}
		}
		return framebuffers;
	}

	// Вектор, в котором будут храниться все объекты
	static std::vector<GameObject> gameObjects;

	CubemapObject cubemapObject;

	static void initBullet(){
		broadphase = new btDbvtBroadphase();
		collisionConfiguration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(collisionConfiguration);
		solver = new btSequentialImpulseConstraintSolver;
		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
	}

	static void cleanBullet(){
		delete solver;
		delete dispatcher;
		delete collisionConfiguration;
		delete broadphase;
		delete dynamicsWorld;
	}

	static void init(GLFWwindow *window){
		instance.createInstance();
		physicalDevice.pickPhysicalDevice(instance.get());
		device.createDevice(physicalDevice.get(),physicalDevice.getQueueIndices());
		surface.createSurface(window, instance.get());
		swapchain.createSwapchain(physicalDevice.get(),device.get(),surface.get(), physicalDevice.getQueueIndices());
		swapchain.createImageViews(device.get());
		commandPool.createCommandPool(physicalDevice.getQueueIndices(),device.get());

		if (MSAAsamples>maxMSAAsamples) {
			MSAAsamples = maxMSAAsamples;
		}

		depthImage.createDepthBuffer(
			device.get(),
			device.getGraphicsQueue(),
			swapchain.getInfo().imageExtent,physicalDevice.get(),
			commandPool.get()
		);

		multisamplingBuffer.createBuffer(
			device.get(),
			physicalDevice.get(),
			swapchain.getInfo().imageExtent,
			swapchain.getInfo().imageFormat
		);

		setLayoutForMesh.createDescriptorSetLayoutForGameObjects(device.get());
		setLayoutForRigidBodyMesh.createDescriptorSetLayoutForGameObjects(device.get());
		setLayoutForCubemapObjects.createDescriptorSetLayoutForCubemapObjects(device.get());
		renderPass.createRenderPass(device.get(),swapchain.getInfo(),depthImage.getDepthFormat());

		graphicsPipelineForMesh.createGraphicsPipeline(
			device.get(),
			swapchain.getInfo(),
			*setLayoutForMesh.pGet(),
			renderPass.getRenderPass()
		);

		graphicsPipelineForCubemapObjects.createGraphicsPipeline(
			device.get(),
			swapchain.getInfo(),
			*setLayoutForCubemapObjects.pGet(),
			renderPass.getRenderPass()

		);

		graphicsPipelineForRigidBodyMesh.createGraphicsPipeline(
			device.get(),
			swapchain.getInfo(),
			*setLayoutForRigidBodyMesh.pGet(),
			renderPass.getRenderPass()
		);

		swapchain.setFramebuffers(
			createFramebuffers(
				device.get(), 
				renderPass.getRenderPass(),
				*swapchain.pGetImageViews(),depthImage.getImageView(),
				multisamplingBuffer.getImageView(),
				swapchain.getInfo().imageExtent
			)
		);

		descriptorPoolForMesh.createDescriptorPoolforGameObjects(
			device.get(), 
			*swapchain.pGetImageViews()
		);

		descriptorPoolForRigidBodyMesh.createDescriptorPoolforGameObjects(
			device.get(),
			*swapchain.pGetImageViews()
		);

		descriptorPoolForImgui.createDescriptorPoolforImgui(
			device.get(),
			*swapchain.pGetImageViews()
		);

		descriptorPoolForCubemapObjects.createDescriptorPoolforCubemapObjects(
			device.get(),
			*swapchain.pGetImageViews()
		);
		// Объявление отображаемых объектов
		{
			initBullet();

			GameObject gameObject;
			gameObject.getMesh()->createMesh("assets/chair.obj");
			gameObject.setId((int)gameObjects.size());
			gameObject.setName("chair");
			gameObject.getMesh()->addBaseColorTexture("assets/chair.png");
			gameObject.getMesh()->transform.scale(glm::vec3(0.2f,0.2f, 0.2f));
			gameObject.getMesh()->transform.initTranslation(glm::vec3(+8.0f, 0.0f, 5.0f));
			gameObjects.push_back(gameObject);

			GameObject gameObject1;
		
			gameObject1.getMesh()->createMesh("assets/table.obj");
			gameObject1.setId((int)gameObjects.size());
			gameObject1.setName("Table");
			gameObject1.getMesh()->addBaseColorTexture("assets/table.jpg");
			gameObject1.getMesh()->transform.initTranslation(glm::vec3(6.0f,0.2f,5.0f));
			gameObjects.push_back(gameObject1);

			GameObject box;
			box.getMesh()->createMesh("assets/cube.obj");
			box.getMesh()->transform.initTranslation(glm::vec3(-15.0f, 5.0f, 0.0f));
			box.getMesh()->transform.initRotation(glm::vec3(0.1f, 90.1f, 15.1f));
			box.getMesh()->addBaseColorTexture("assets/texture1.jpg");
			
			box.setId((int)gameObjects.size());
			box.setName("box1");
			box.getRigidBody()->createRigidBodyFromMesh(
				*box.getMesh(),
				1.1f,
				1.0f,
				1.9f,
				dynamicsWorld,
				box.getId()
			);
			box.getMesh()->transform.scale(glm::vec3(1.5f, 1.5f, 1.5f));
			box.getRigidBody()->setRigidBodyTransformFromMesh(*box.getMesh());
			gameObjects.push_back(box);

			GameObject box2;
			box2.getMesh()->createMesh("assets/cube.obj");
			box2.setId((int)gameObjects.size());
			box2.setName("box2");
			box2.getMesh()->addBaseColorTexture("assets/texture1.jpg");
			box2.getMesh()->transform.initTranslation(glm::vec3(+-10.0f,4.0f, 1.0f));
			box2.getMesh()->transform.scale(glm::vec3(1.5f, 1.5f, 1.5f));
			box2.getMesh()->transform.initRotation(glm::vec3(0.1f, 90.1f, 15.1f));
			box2.getRigidBody()->createRigidBodyFromMesh(
				*box2.getMesh(),
				1.1f,
				1.0f,
				1.9f,
				dynamicsWorld,
				box2.getId()
			);
			box2.getMesh()->transform.scale(glm::vec3(1.5f, 1.5f, 1.5f));
			box2.getRigidBody()->setRigidBodyTransformFromMesh(*box2.getMesh());
			gameObjects.push_back(box2);

			GameObject box3;
			box3.getMesh()->createMesh("assets/cube.obj");
			box3.setId((int)gameObjects.size());
			box3.setName("box3");
			box3.getMesh()->addBaseColorTexture("assets/texture1.jpg");
			box3.getMesh()->transform.initTranslation(glm::vec3(0.0f,5.0f, 0.0f));
			box3.getMesh()->transform.scale(glm::vec3(1.5f,1.5f,1.5f));
			box3.getMesh()->transform.initRotation(glm::vec3(20.1f, 90.1f, 15.1f));
			box3.getRigidBody()->createRigidBodyFromMesh(
				*box3.getMesh(),
				1.1f,
				1.0f,
				1.9f,
				dynamicsWorld,
				box3.getId()	
			);
			box3.getRigidBody()->setRigidBodyTransformFromMesh(*box3.getMesh());
			gameObjects.push_back(box3);

			GameObject gameObject2;
			gameObject2.getMesh()->createMesh("assets/floor.obj");
			gameObject2.setId((int)gameObjects.size());
			gameObject2.setName("Floor");
			gameObject2.getMesh()->addBaseColorTexture("assets/ceramic.jpg");
			gameObject2.getMesh()->transform.scale(glm::vec3(4.0f, 1.0f, 4.0f));
			gameObject2.getRigidBody()->createStaticRigidBodyWithPlaneShape(
				dynamicsWorld,
				gameObject2.getId()
			);
			gameObject2.getRigidBody()->setRigidBodyTransformFromMesh(*gameObject2.getMesh());
			gameObjects.push_back(gameObject2);
		}
		syncObjects.createSyncObjects	     (device.get());
		
		std::vector<std::string> paths= {
			"assets/skybox/right.jpg",
			"assets/skybox/left.jpg",
			"assets/skybox/top.jpg",
			"assets/skybox/bottom.jpg",
			"assets/skybox/front.jpg",
			"assets/skybox/back.jpg"
		};

		cubemapObject.init(
			paths
		);
	}
	
	CommandBuffer drawCommandBuffer;
	static void DrawObjects(GLFWwindow *window, ImDrawData *drawData, bool isEditorEnabled)
	{
		uint32_t imageIndex;
		vkWaitForFences(device.get(), 1, &syncObjects.getFences()[currentFrame], VK_TRUE, 100000);
		
		VkResult result;
		result = vkAcquireNextImageKHR(device.get(), swapchain.get(), UINT64_MAX,  syncObjects.getImageAvailableSemaphores()[currentFrame],VK_NULL_HANDLE,  &imageIndex);

		//if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		//	recreateSwapchain(window);
		//	return;
		//}

		//if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {s
		//	vkWaitForFences(device_Device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		//}

		//imagesInFlight[imageIndex] = syncFences[currentFrame];

		//----------------Обновляем uniform'ы--------------------------------------
		for (size_t i = 0; i < gameObjects.size(); i++) {
			gameObjects[i].updateUniformBuffers(imageIndex, device.get(), swapchain.getInfo().imageExtent);
			
		}
		cubemapObject.updateUniformBuffers(imageIndex, device.get(), swapchain.getInfo().imageExtent);
		//-------------------------------------------------------------------------

		VkPipelineStageFlags stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore waitSemaphores[] = { syncObjects.getImageAvailableSemaphores()[currentFrame] };
		VkSemaphore signalSemaphores[] = { syncObjects.getImageRenderedSemaphores()[currentFrame] };

		drawCommandBuffer.beginCommandBuffer(device.get(), commandPool.get());

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass.getRenderPass();
		renderPassBeginInfo.renderArea.extent = swapchain.getInfo().imageExtent;
		renderPassBeginInfo.renderArea.offset = { 0,0 };
		renderPassBeginInfo.framebuffer = swapchain.getSwapchainFramebuffers()[imageIndex];

		VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
		VkClearValue depthClearColor = { 1.0,0.0f };
		VkClearValue resolveColor = { 0.0f,0.0f,0.0f,1.0f };
		VkClearValue clearColors[] = {
			clearColor, depthClearColor, resolveColor
		};
		renderPassBeginInfo.pClearValues = clearColors;
		renderPassBeginInfo.clearValueCount = 3;

		vkCmdBeginRenderPass(drawCommandBuffer.get(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		cubemapObject.recordDrawCalls(drawCommandBuffer.get(),imageIndex);

		for (size_t i = 0; i < gameObjects.size(); i++) {
			gameObjects[i].recordDrawCalls(
				drawCommandBuffer.get(),
				imageIndex
			);
		}

		if (isEditorEnabled) {
			ImGui_ImplVulkan_RenderDrawData(drawData, drawCommandBuffer.get());
		}

		vkCmdEndRenderPass(drawCommandBuffer.get());

		drawCommandBuffer.endCommandBuffer();

		VkCommandBuffer singleTimePointer[] = { drawCommandBuffer.get() };
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = (uint32_t)1;
		submitInfo.pCommandBuffers = singleTimePointer;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pWaitDstStageMask = stages;

		vkResetFences(device.get(), 1, &syncObjects.getFences()[currentFrame]);
		if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, syncObjects.getFences()[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit info");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.waitSemaphoreCount = 1;

		VkSwapchainKHR swapchains[] = { swapchain.get() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pResults = 0;
		presentInfo.pImageIndices = &imageIndex;

		//Используется одна очередь для отрисовки и вывода изображения
		result = vkQueuePresentKHR(device.getGraphicsQueue(), &presentInfo);
		//if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		//	recreateSwapchain(window);
		//	return;
		//}
		

		currentFrame = (currentFrame + 1) % MAX_FRAMES;
	}

	static void clear(){
		vkDeviceWaitIdle(device.get());
		drawCommandBuffer.freeCommandBuffer(device.get(), commandPool.get());
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		setLayoutForMesh.destroy(device.get());
		setLayoutForCubemapObjects.destroy(device.get());
		setLayoutForRigidBodyMesh.destroy(device.get());
		depthImage.destroy(device.get());
		for (size_t i = 0; i < gameObjects.size(); i++) {
			gameObjects[i].destroy();	
		}
		cubemapObject.destroy();
		descriptorPoolForMesh.destroy(device.get());
		descriptorPoolForRigidBodyMesh.destroy(device.get());
		descriptorPoolForImgui.destroy(device.get());
		descriptorPoolForCubemapObjects.destroy(device.get());
		syncObjects.destroySyncObjects(device.get());
		renderPass.destroy(device.get());
		multisamplingBuffer.destroy(device.get());
		graphicsPipelineForMesh.destroyPipelineObjects(device.get());
		graphicsPipelineForCubemapObjects.destroyPipelineObjects(device.get());
		graphicsPipelineForRigidBodyMesh.destroyPipelineObjects(device.get());
		commandPool.destroy(device.get());
		swapchain.destroySwapchainObjects(device.get());
		surface.destroy(instance.get());
		device.destroy();
		instance.destroy();
		cleanBullet();
	}  

	glm::vec3 castRay(double mouseX, double mouseY){
		double normalizedX = 2*mouseX/WIDTH - 1;
		double normalizedY = 2*mouseY / HEIGHT - 1;
		glm::vec4 deviceCoord = glm::vec4(normalizedX, normalizedY,-1.0f,1.0f);
		glm::mat4 invertedProj = glm::inverse(glm::perspective(glm::radians(80.0f), (WIDTH / (float)HEIGHT), 0.1f, 1000.0f));
		glm::vec4 eyeCoords = invertedProj * deviceCoord;
		eyeCoords = glm::vec4(eyeCoords.x,eyeCoords.y,-1.0f,0.0f);
		glm::mat4 viewMatrix = camera.getView();
		eyeCoords = glm::inverse(viewMatrix) * eyeCoords;
		return glm::normalize(glm::vec3(eyeCoords.x,eyeCoords.y,eyeCoords.z));
	}
}
