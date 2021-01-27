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
#include    "../vendor/imgui_impl_win32.h"
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
#include    "Events.h"
#include	"Device.h"
#include	"Buffers.h"
#include	"CommandBuffer.h"
#include	"CommandPool.h"
#include	"DescriptorPools.h"
#include	"DescriptorSetLayouts.h"
#include	"Entities.h"
#include	"Pipelines.h"
#include	"Surface.h"
#include	"Swapchain.h"
#include	"SyncObjects.h"
#include	"Images.h"
#include    "Scene.h"


void imguiErrFunction(const VkResult result) {
	if (result != VK_SUCCESS) {
		std::cout << "Imgui vulkan errcode: " << result << std::endl;
		throw std::runtime_error("Imgui fatal error");
	}
}


namespace Lava{
	static uint32_t currentFrame = 0;
	static Camera main_ActiveCamera;
	//Camera debugCamera;
	//Camera activeCamera;

//  Настройка и создание фреймбуферов
	static std::vector<VkFramebuffer> createFramebuffers(VkDevice device, VkRenderPass renderPass, std::vector<VkImageView> swapchainImageViews,
		VkImageView depthImageView, VkImageView multisamplingImageView, VkExtent2D swapchainExtent) {
	
		std::vector<VkFramebuffer> framebuffers;
		framebuffers.resize(swapchainImageViews.size());
		for (size_t i = 0; i < swapchainImageViews.size(); i++) {
			VkImageView attachments[] = {
			//  Пост обработка (анизотропная фильтрация) 
				multisamplingImageView,
			//  Пост обработка (Z-буфер) 
				depthImageView,
			//  Основное изображение
				swapchainImageViews[i],
			};
			VkFramebufferCreateInfo 
			framebufferCreateInfo{};
			framebufferCreateInfo.renderPass = renderPass;

		//  Размер фреймбуфера указывается в соответствии с размером изображения,
		//  получаемым из свапчейна
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

//  Буфер с коммандами для отрисовки примитивов
	CommandBuffer drawCommandBuffer;

//  Инициализация объектов Vulkan
	static void initVulkan(HWND hwnd, HINSTANCE hInstance){
    //  Создание сеанса Vulkan
		gInstance.createInstance();
	//  Выбор физического устройства
		gPhysicalDevice.PickPhysicalDevice(gInstance.get());
	//  Создание логического устройства
		gDevice.CreateDevice(gPhysicalDevice.Get(), gPhysicalDevice.GetQueueIndices());
	//  Выбор поверхности для вывода (В данном случае окно win32)
		gSurface.CreateSurface(hwnd, hInstance, gInstance.get());

	//  задание размеров viewport'а с отрисованными примитивами
		gEditor3DViewport.x = 0;
		gEditor3DViewport.y = 0;
		gEditor3DViewport.height = gHeight;
		gEditor3DViewport.width = gWidth;
		gEditor3DViewport.minDepth = 0.0f;
		gEditor3DViewport.maxDepth = 1.0f;
		
	//  Создание свапчейна 
		gSwapchain.CreateSwapchain(gPhysicalDevice.Get(), 
			gDevice.Get(), 
			gSurface.Get(), 
			gPhysicalDevice.GetQueueIndices());

		gSwapchain.CreateImageViews(gDevice.Get());

	//  Создание пула комманд
		gCommandPool.CreateCommandPool(gPhysicalDevice.GetQueueIndices(), 
												   gDevice.Get());
	
	//  задание размеров отсечения viewport'а с отрисованными примитивами
	//  в данном случае отрисовывается весь viewport
		gEditor3DScissors.extent = gSwapchain.GetInfo().imageExtent;

	//  Проверка на превышение максимального уровня сглаживания
		if (gMSAAsamples>gmaxMSAAsamples) {
			gMSAAsamples = gmaxMSAAsamples;
		}

	//  Создание Z буфера
		gDepthImage.CreateDepthBuffer(
			gDevice.Get(),
			gDevice.GetGraphicsQueue(),
			gSwapchain.GetInfo().imageExtent, gPhysicalDevice.Get(),
			gCommandPool.Get()
		);

	//  Создание буфера с расчетом сглаживания
		gMultisamplingBuffer.CreateBuffer(
			gDevice.Get(),
			gPhysicalDevice.Get(),
			gSwapchain.GetInfo().imageExtent,
			gSwapchain.GetInfo().imageFormat
		);

	//  
		gSetLayoutForMesh.CreateDescriptorSetLayout(gDevice.Get());
	//
		gSetLayoutForRigidBodyMesh.CreateDescriptorSetLayout(gDevice.Get());
	//
		gSetLayoutForCubemapObjects.CreateDescriptorSetLayout(gDevice.Get());

		gRenderPass.CreateRenderPass(gDevice.Get(), 
												  gSwapchain.GetInfo(), 
												  gDepthImage.GetDepthFormat());

	//  Создание пайплайна для отрисовки моделей
		gGraphicsPipelineForMesh.CreateGraphicsPipeline(
			gDevice.Get(),
			gSwapchain.GetInfo(),
			*gSetLayoutForMesh.PGet(),
			gRenderPass.GetRenderPass()
		);

	//  Создание пайплайна для отрисовки скайбокса
		gGraphicsPipelineForCubemapObjects.CreateGraphicsPipeline(
			gDevice.Get(),
			gSwapchain.GetInfo(),
			*gSetLayoutForCubemapObjects.PGet(),
			gRenderPass.GetRenderPass()
		);

	//  Создание пайплайна для отрисовки моделей с помощью линий 
	//  (Данный пайплайн используется для отладки в физики твердых тел)
		gGraphicsPipelineForRigidBodyMesh.CreateGraphicsPipeline(
			gDevice.Get(),
			gSwapchain.GetInfo(),
			*gSetLayoutForRigidBodyMesh.PGet(),
			gRenderPass.GetRenderPass()
		);

	
	//  Создание фреймбуферов
		gSwapchain.SetFramebuffers(
			createFramebuffers(
				gDevice.Get(),
				gRenderPass.GetRenderPass(),
				*gSwapchain.PGetImageViews(), gDepthImage.GetImageView(),
				gMultisamplingBuffer.GetImageView(),
				gSwapchain.GetInfo().imageExtent
			)
		);


		gDescriptorPoolForMesh.CreateDescriptorPool(
			gDevice.Get(),
			*gSwapchain.PGetImageViews()
		);

		gDescriptorPoolForRigidBodyMesh.CreateDescriptorPool(
			gDevice.Get(),
			*gSwapchain.PGetImageViews()
		);

		gDescriptorPoolForImgui.CreateDescriptorPool(
			gDevice.Get(),
			*gSwapchain.PGetImageViews()
		);

		gDescriptorPoolForCubemapObjects.CreateDescriptorPool(
			gDevice.Get(),
			*gSwapchain.PGetImageViews()
		);

	 
		gSyncObjects.CreateSyncObjects(gDevice.Get());

	}

	

	
//  Пересоздание свапчейна. 
//	Используется для обновления размеров выводимого изображения
//  при изменении размеров окна.
	static void recreateSwapchain() {
		vkDeviceWaitIdle(gDevice.Get());

		gDepthImage.Destroy(gDevice.Get());
		gMultisamplingBuffer.Destroy(gDevice.Get());

		gSwapchain.DestroySwapchainObjects(gDevice.Get());

		gSwapchain.CreateSwapchain(gPhysicalDevice.Get(), 
											  gDevice.Get(), 
											  gSurface.Get(), 
											  gPhysicalDevice.GetQueueIndices());

		gSwapchain.CreateImageViews(gDevice.Get());

		gDepthImage.CreateDepthBuffer(
			gDevice.Get(),
			gDevice.GetGraphicsQueue(),
			gSwapchain.GetInfo().imageExtent, gPhysicalDevice.Get(),
			gCommandPool.Get()
		);

		gMultisamplingBuffer.CreateBuffer(
			gDevice.Get(),
			gPhysicalDevice.Get(),
			gSwapchain.GetInfo().imageExtent,
			gSwapchain.GetInfo().imageFormat
		);

		gSwapchain.SetFramebuffers(
			createFramebuffers(
				gDevice.Get(),
				gRenderPass.GetRenderPass(),
				*gSwapchain.PGetImageViews(), gDepthImage.GetImageView(),
				gMultisamplingBuffer.GetImageView(),
				gSwapchain.GetInfo().imageExtent
			)
		);
	}

//  Данная функция отрисовывает все примитивы на экран
	static void DrawScene(ImDrawData *drawData, Scene &scene)
	{
	
		uint32_t imageIndex;
		vkWaitForFences(gDevice.Get(), 1, &gSyncObjects.GetFences()[currentFrame], VK_TRUE, 100000);
		
		VkResult result;
		result = vkAcquireNextImageKHR(gDevice.Get(), 
									   gSwapchain.Get(), 
									   UINT64_MAX, 
									   gSyncObjects.GetImageAvailableSemaphores()[currentFrame],
									   VK_NULL_HANDLE,  
									   &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapchain();
			return;
		}


		//----------------Обновляем uniform'ы--------------------------------------
		for (size_t i = 0; i < scene.GetVectorOfEntities().size(); i++) {
			scene.GetVectorOfEntities()[i]->UpdateUniforms(imageIndex, gDevice.Get());
		}


		VkPipelineStageFlags stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore waitSemaphores[] = { gSyncObjects.GetImageAvailableSemaphores()[currentFrame] };
		VkSemaphore signalSemaphores[] = { gSyncObjects.GetImageRenderedSemaphores()[currentFrame] };

		VkRenderPassBeginInfo 
		renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = gRenderPass.GetRenderPass();
		renderPassBeginInfo.renderArea.extent = gSwapchain.GetInfo().imageExtent;
		renderPassBeginInfo.renderArea.offset = { 0,0 };
		renderPassBeginInfo.framebuffer = gSwapchain.GetSwapchainFramebuffers()[imageIndex];

		VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
		VkClearValue depthClearColor = { 1.0,0.0f };
		VkClearValue resolveColor = { 0.0f,0.0f,0.0f,1.0f };
		VkClearValue clearColors[] = {
			clearColor, depthClearColor, resolveColor
		};
		renderPassBeginInfo.pClearValues = clearColors;
		renderPassBeginInfo.clearValueCount = 3;


		drawCommandBuffer.AllocateCommandBuffer(gDevice.Get(), gCommandPool.Get());
		drawCommandBuffer.BeginCommandBuffer();
	
	//  Начало рендерпасса
	//  (Рендерпасс отвечает за пост обработку)
		vkCmdBeginRenderPass(drawCommandBuffer.Get(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	//  Передача комманд для отрисовки всех игровых объектов в буфер с коммандами
		for (size_t i = 0; i < scene.GetVectorOfEntities().size(); i++) {
			scene.GetVectorOfEntities()[i]->Draw(
				drawCommandBuffer.Get(),
				imageIndex
			);
		}

		if (LAVA_GLOBAL_ENABLE_IMGUI) {
			ImGui_ImplVulkan_RenderDrawData(drawData, drawCommandBuffer.Get());
		}

		vkCmdEndRenderPass(drawCommandBuffer.Get());

		drawCommandBuffer.EndCommandBuffer();

		VkCommandBuffer singleTimePointer[] = { drawCommandBuffer.Get() };
		VkSubmitInfo 
		submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = (uint32_t)1;
		submitInfo.pCommandBuffers = singleTimePointer;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pWaitDstStageMask = stages;

		vkResetFences(gDevice.Get(), 1, &gSyncObjects.GetFences()[currentFrame]);
	
	//  Выполнение всех комманд из буфера с коммандами
		if (vkQueueSubmit(gDevice.GetGraphicsQueue(), 1, &submitInfo, gSyncObjects.GetFences()[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit info");
		}

		VkPresentInfoKHR 
		presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.waitSemaphoreCount = 1;

		VkSwapchainKHR 
		swapchains[] = { gSwapchain.Get() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pResults = 0;
		presentInfo.pImageIndices = &imageIndex;

	//  Результат выполненных коммамнд выводится на экран
		result = vkQueuePresentKHR(gDevice.GetGraphicsQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreateSwapchain();
			return;
		}
		currentFrame = (currentFrame + 1) % gmax_frames;
	}

	static void clear(){
		
		drawCommandBuffer.FreeCommandBuffer(gDevice.Get(), gCommandPool.Get());
		
		gSetLayoutForMesh.Destroy(gDevice.Get());
		gSetLayoutForCubemapObjects.Destroy(gDevice.Get());
		gSetLayoutForRigidBodyMesh.Destroy(gDevice.Get());
		gDepthImage.Destroy(gDevice.Get());
		//cubemapObject.destroy();
		gDescriptorPoolForMesh._Destroy_(gDevice.Get());
		gDescriptorPoolForRigidBodyMesh._Destroy_(gDevice.Get());
		gDescriptorPoolForImgui._Destroy_(gDevice.Get());
		gDescriptorPoolForCubemapObjects._Destroy_(gDevice.Get());
		gSyncObjects.DestroySyncObjects(gDevice.Get());
		gRenderPass.Destroy(gDevice.Get());
		gMultisamplingBuffer.Destroy(gDevice.Get());
		gGraphicsPipelineForMesh.DestroyPipelineObjects(gDevice.Get());
		gGraphicsPipelineForCubemapObjects.DestroyPipelineObjects(gDevice.Get());
		gGraphicsPipelineForRigidBodyMesh.DestroyPipelineObjects(gDevice.Get());
		gCommandPool.Destroy(gDevice.Get());
		gSwapchain.DestroySwapchainObjects(gDevice.Get());
		gSurface.Destroy(gInstance.get());
		gDevice.Destroy();
		gInstance.destroy();
		//cleanBullet();
	}  

	glm::vec3 castRay(double mouseX,double mouseY){
		double normalizedX = 2*mouseX/gWidth - 1;
		double normalizedY = 2*mouseY / gHeight - 1;
		glm::vec4 deviceCoord = glm::vec4(normalizedX, normalizedY,-1.0f,1.0f);
		glm::mat4 invertedProj = glm::inverse(glm::perspective(glm::radians(80.0f), (gWidth / (float)gHeight), 0.1f, 1000.0f));
		glm::vec4 eyeCoords = invertedProj * deviceCoord;
		eyeCoords = glm::vec4(eyeCoords.x,eyeCoords.y,-1.0f,0.0f);
		glm::mat4 viewMatrix = debugCamera.GetView();
		eyeCoords = glm::inverse(viewMatrix) * eyeCoords;
		return glm::normalize(glm::vec3(eyeCoords.x,eyeCoords.y,eyeCoords.z));
	}
}
