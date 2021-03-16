#ifndef mainvulkan_h
#define mainvulkan_h

#define	VK_NO_PROTOTYPES
#define	GLM_FORCE_RADIANS

#include "../../vendor/imgui.h"
#include "../../vendor/imgui_impl_glfw.h"
#include "../../vendor/imgui_impl_vulkan.h"
#include "../../vendor/imgui_impl_win32.h"
#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/quaternion.hpp"
#include "../../vendor/glm/gtx/quaternion.hpp"
#include "../../vendor/volk.h"

#include <array>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

#include "Instance.h"
#include "PhysicalDevice.h"
#include "DataTypes.h"
#include "Events.h"
#include "Device.h"
#include "Buffers.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorPools.h"
#include "DescriptorSetLayouts.h"
#include "Entities.h"
#include "Pipelines.h"
#include "Surface.h"
#include "Swapchain.h"
#include "SyncObjects.h"
#include "Images.h"
#include "Scene.h"


static void imguiErrFunction(const VkResult result) {
	if (result != VK_SUCCESS) {
		std::cout << "Imgui vulkan errcode: " << result << std::endl;
		throw std::runtime_error("Imgui fatal error");
	}
}


namespace Engine{
	static uint32_t currentFrame = 0;
	static Camera main_ActiveCamera;

	//  Настройка и создание фреймбуферов
	static std::vector<VkFramebuffer> createFramebuffers(VkDevice device, VkRenderPass renderPass, std::vector<VkImageView> swapchainImageViews,
		VkImageView depthImageView, VkImageView multisamplingImageView, VkExtent2D swapchainExtent) {
	
		std::vector<VkFramebuffer> framebuffers;
		framebuffers.resize(swapchainImageViews.size());
		for (size_t i = 0; i < swapchainImageViews.size(); i++) {
			//	Изображение во фреймбуфере
			VkImageView attachments[] = {
			//  msaa
				multisamplingImageView,
			//  Z-буфер
				depthImageView,
			//  Изображение из swapchain
				swapchainImageViews[i],
			};

			VkFramebufferCreateInfo framebufferCreateInfo{};
			
			//  Размер фреймбуфера указывается в соответствии с размером изображения,
			//  получаемым из свапчейна
			{
				framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCreateInfo.renderPass = renderPass;
				framebufferCreateInfo.height = swapchainExtent.height;
				framebufferCreateInfo.width = swapchainExtent.width;
				framebufferCreateInfo.pAttachments = attachments;
				framebufferCreateInfo.attachmentCount = (uint32_t)3;
				framebufferCreateInfo.layers = 1;
			}
			
			if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer");
			}
		}
		return framebuffers;
	}

//  Буфер с коммандами для отрисовки примитивов
	static CommandBuffer drawCommandBuffer;

		//  Инициализация объектов Vulkan
	static void initVulkan(HWND hwnd, HINSTANCE hInstance){
		
		Globals::gInstance.createInstance();

		Globals::gPhysicalDevice.PickPhysicalDevice(Globals::gInstance.get());
	
		Globals::gDevice.CreateDevice(Globals::gPhysicalDevice.Get(), Globals::gPhysicalDevice.GetQueueIndices());
		
		Globals::gSurface.CreateSurface(hwnd, hInstance, Globals::gInstance.get());

		Globals::gEditor3DView.x = 0;
		Globals::gEditor3DView.y = 0;
		Globals::gEditor3DView.height = static_cast<float>(Globals::gHeight);
		Globals::gEditor3DView.width = static_cast<float>(Globals::gWidth);
		Globals::gEditor3DView.minDepth = 0.0f;
		Globals::gEditor3DView.maxDepth = 1.0f;
		
		Globals::gSwapchain.CreateSwapchain(Globals::gPhysicalDevice.Get(),
			Globals::gDevice.Get(),
			Globals::gSurface.Get(),
			Globals::gPhysicalDevice.GetQueueIndices());

		Globals::gSwapchain.CreateImageViews(Globals::gDevice.Get());

		Globals::gCommandPool.CreateCommandPool(Globals::gPhysicalDevice.GetQueueIndices(),
			Globals::gDevice.Get());
	
		Globals::gEditor3DScissors.extent = Globals::gSwapchain.GetInfo().imageExtent;


		Globals::gDepthImage.CreateDepthBuffer(
			Globals::gDevice.Get(),
			Globals::gDevice.GetGraphicsQueue(),
			Globals::gSwapchain.GetInfo().imageExtent, Globals::gPhysicalDevice.Get(),
			Globals::gCommandPool.Get()
		);

		Globals::gMultisamplingBuffer.CreateBuffer(
			Globals::gDevice.Get(),
			Globals::gPhysicalDevice.Get(),
			Globals::gSwapchain.GetInfo().imageExtent,
			Globals::gSwapchain.GetInfo().imageFormat
		);

	 
		Globals::gSetLayoutForMesh.CreateDescriptorSetLayout(Globals::gDevice.Get());
	
		Globals::gSetLayoutForRigidBodyMesh.CreateDescriptorSetLayout(Globals::gDevice.Get());
	
		Globals::gSetLayoutForCubemapObjects.CreateDescriptorSetLayout(Globals::gDevice.Get());

		Globals::gRenderPass.CreateRenderPass(Globals::gDevice.Get(),
			Globals::gSwapchain.GetInfo(),
			Globals::gDepthImage.GetDepthFormat());

		Globals::gGraphicsPipelineForMesh.CreateGraphicsPipeline(
			Globals::gDevice.Get(),
			Globals::gSwapchain.GetInfo(),
			Globals::gSetLayoutForMesh.Get(),
			Globals::gRenderPass.GetRenderPass()
		);

		Globals::gGraphicsPipelineForCubemapObjects.CreateGraphicsPipeline(
			Globals::gDevice.Get(),
			Globals::gSwapchain.GetInfo(),
			Globals::gSetLayoutForCubemapObjects.Get(),
			Globals::gRenderPass.GetRenderPass()
		);

		Globals::gGraphicsPipelineForRigidBodyMesh.CreateGraphicsPipeline(
			Globals::gDevice.Get(),
			Globals::gSwapchain.GetInfo(),
			Globals::gSetLayoutForRigidBodyMesh.Get(),
			Globals::gRenderPass.GetRenderPass()
		);

		Globals::gSwapchain.SetFramebuffers(
			createFramebuffers(
				Globals::gDevice.Get(),
				Globals::gRenderPass.GetRenderPass(),
				*Globals::gSwapchain.PGetImageViews(), 
				Globals::gDepthImage.GetImageView(),
				Globals::gMultisamplingBuffer.GetImageView(),
				Globals::gSwapchain.GetInfo().imageExtent
			)
		);

		Globals::gDescriptorPoolForMesh.CreateDescriptorPool(
			Globals::gDevice.Get(),
			*Globals::gSwapchain.PGetImageViews()
		);

		Globals::gDescriptorPoolForRigidBodyMesh.CreateDescriptorPool(
			Globals::gDevice.Get(),
			*Globals::gSwapchain.PGetImageViews()
		);

		Globals::gDescriptorPoolForImgui.CreateDescriptorPool(
			Globals::gDevice.Get(),
			*Globals::gSwapchain.PGetImageViews()
		);

		Globals::gDescriptorPoolForCubemapObjects.CreateDescriptorPool(
			Globals::gDevice.Get(),
		   *Globals::gSwapchain.PGetImageViews()
		);

		Globals::gSyncObjects.CreateSyncObjects(Globals::gDevice.Get());

	}

	
	static void recreateSwapchain() {
		vkDeviceWaitIdle(Globals::gDevice.Get());
		Globals::gDepthImage.Destroy(Globals::gDevice.Get());
		Globals::gMultisamplingBuffer.Destroy(Globals::gDevice.Get());

		Globals::gSwapchain.DestroySwapchainObjects(Globals::gDevice.Get());

		Globals::gSwapchain.CreateSwapchain(Globals::gPhysicalDevice.Get(),
			Globals::gDevice.Get(),
			Globals::gSurface.Get(),
			Globals::gPhysicalDevice.GetQueueIndices());

		Globals::gSwapchain.CreateImageViews(Globals::gDevice.Get());

		Globals::gDepthImage.CreateDepthBuffer(
			Globals::gDevice.Get(),
			Globals::gDevice.GetGraphicsQueue(),
			Globals::gSwapchain.GetInfo().imageExtent, Globals::gPhysicalDevice.Get(),
			Globals::gCommandPool.Get()
		);

		Globals::gMultisamplingBuffer.CreateBuffer(
			Globals::gDevice.Get(),
			Globals::gPhysicalDevice.Get(),
			Globals::gSwapchain.GetInfo().imageExtent,
			Globals::gSwapchain.GetInfo().imageFormat
		);

		Globals::gSwapchain.SetFramebuffers(
			createFramebuffers(
				Globals::gDevice.Get(),
				Globals::gRenderPass.GetRenderPass(),
				*Globals::gSwapchain.PGetImageViews(), Globals::gDepthImage.GetImageView(),
				Globals::gMultisamplingBuffer.GetImageView(),
				Globals::gSwapchain.GetInfo().imageExtent
			)
		);
		Engine::Globals::gEditor3DScissors.extent = Engine::Globals::gSwapchain.GetInfo().imageExtent;

	}

	static void DrawScene(ImDrawData *drawData, Scene *scene)
	{
		uint32_t imageIndex;
		vkWaitForFences(Globals::gDevice.Get(), 1, &Globals::gSyncObjects.GetFences()[currentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(Globals::gDevice.Get(), 1, &Globals::gSyncObjects.GetFences()[currentFrame]);

		VkResult result;
		result = vkAcquireNextImageKHR(
			Globals::gDevice.Get(),
			Globals::gSwapchain.Get(),
			UINT64_MAX, 
			Globals::gSyncObjects.GetImageAvailableSemaphores()[currentFrame],
			VK_NULL_HANDLE,  
			&imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreateSwapchain();
			return;
		}

		
		for (size_t i = 0; i < scene->GetVectorOfEntities().size(); i++) {
			if (scene->GetVectorOfEntities()[i]->GetEntityType() == ENTITY_TYPE_GAME_OBJECT){
				((GameObject*)scene->GetVectorOfEntities()[i])->UpdateUniforms(imageIndex, Globals::gDevice.Get(),scene->GetVectorOfSpotlightAttributes());
			}
			if (scene->GetVectorOfEntities()[i]->GetEntityType() == ENTITY_TYPE_CUBEMAP_OBJECT){
				((CubemapObject*)scene->GetVectorOfEntities()[i])->UpdateUniforms(imageIndex, Globals::gDevice.Get());
			}
			if (scene->GetVectorOfEntities()[i]->GetEntityType() == ENTITY_TYPE_SPOTLIGHT_OBJECT) {
				((SpotlightObject*)scene->GetVectorOfEntities()[i])->UpdateUniforms(imageIndex, Globals::gDevice.Get());
			}
		}


		VkPipelineStageFlags stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore waitSemaphores[] = { Globals::gSyncObjects.GetImageAvailableSemaphores()[currentFrame] };
		VkSemaphore signalSemaphores[] = { Globals::gSyncObjects.GetImageRenderedSemaphores()[currentFrame] };

		VkRenderPassBeginInfo 
		renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = Globals::gRenderPass.GetRenderPass();
		renderPassBeginInfo.renderArea.extent = Globals::gSwapchain.GetInfo().imageExtent;
		renderPassBeginInfo.renderArea.offset = { 0,0 };
		renderPassBeginInfo.framebuffer = Globals::gSwapchain.GetSwapchainFramebuffers()[imageIndex];

		VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
		VkClearValue depthClearColor = { 1.0,0.0f };
		VkClearValue resolveColor = { 0.0f,0.0f,0.0f,1.0f };
		VkClearValue clearColors[] = {
			clearColor, depthClearColor, resolveColor
		};

		renderPassBeginInfo.pClearValues = clearColors;
		renderPassBeginInfo.clearValueCount = 3;

		drawCommandBuffer.AllocateCommandBuffer(Globals::gDevice.Get(), 
			Globals::gCommandPool.Get());

		drawCommandBuffer.BeginCommandBuffer();
	
		vkCmdBeginRenderPass(drawCommandBuffer.Get(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		for (size_t i = 0; i < scene->GetVectorOfEntities().size(); i++) {
			scene->GetVectorOfEntities()[i]->Draw(
				drawCommandBuffer.Get(),
				imageIndex
			);
		}

		if (ENABLE_IMGUI) {
			ImGui_ImplVulkan_RenderDrawData(drawData, drawCommandBuffer.Get());
		}

		vkCmdEndRenderPass(drawCommandBuffer.Get());

		drawCommandBuffer.EndCommandBuffer();

		VkCommandBuffer commandBuffers[] = { drawCommandBuffer.Get() };

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = (uint32_t)1;
		submitInfo.pCommandBuffers = commandBuffers;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pWaitDstStageMask = stages;
	
		if (vkQueueSubmit(Globals::gDevice.GetGraphicsQueue(), 1, &submitInfo, Globals::gSyncObjects.GetFences()[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit info");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.waitSemaphoreCount = 1;

		VkSwapchainKHR swapchains[] = { Globals::gSwapchain.Get() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pResults = 0;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(Globals::gDevice.GetGraphicsQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreateSwapchain();
			return;
		}

		currentFrame = (currentFrame + 1) % Globals::gmax_frames;
	}

	static void clear(){
		drawCommandBuffer.FreeCommandBuffer(Globals::gDevice.Get(), Globals::gCommandPool.Get());
		Globals::gSetLayoutForMesh.Destroy(Globals::gDevice.Get());
		Globals::gSetLayoutForCubemapObjects.Destroy(Globals::gDevice.Get());
		Globals::gSetLayoutForRigidBodyMesh.Destroy(Globals::gDevice.Get());
		Globals::gDepthImage.Destroy(Globals::gDevice.Get());
		Globals::gDescriptorPoolForMesh.Destroy(Globals::gDevice.Get());
		Globals::gDescriptorPoolForRigidBodyMesh.Destroy(Globals::gDevice.Get());
		Globals::gDescriptorPoolForImgui.Destroy(Globals::gDevice.Get());
		Globals::gDescriptorPoolForCubemapObjects.Destroy(Globals::gDevice.Get());
		Globals::gSyncObjects.DestroySyncObjects(Globals::gDevice.Get());
		Globals::gRenderPass.Destroy(Globals::gDevice.Get());
		Globals::gMultisamplingBuffer.Destroy(Globals::gDevice.Get());
		Globals::gGraphicsPipelineForMesh.DestroyPipelineObjects(Globals::gDevice.Get());
		Globals::gGraphicsPipelineForCubemapObjects.DestroyPipelineObjects(Globals::gDevice.Get());
		Globals::gGraphicsPipelineForRigidBodyMesh.DestroyPipelineObjects(Globals::gDevice.Get());
		Globals::gCommandPool.Destroy(Globals::gDevice.Get());
		Globals::gSwapchain.DestroySwapchainObjects(Globals::gDevice.Get());
		Globals::gSurface.Destroy(Globals::gInstance.get());
		Globals::gDevice.Destroy();
		Globals::gInstance.destroy();
	}  

	static glm::vec3 castRay(double mouseX,double mouseY){
		double normalizedX = 2*mouseX/ Globals::gWidth - 1;
		double normalizedY = 2*mouseY / Globals::gHeight - 1;
		glm::vec4 deviceCoord = glm::vec4(normalizedX, normalizedY,-1.0f,1.0f);
		glm::mat4 invertedProj = glm::inverse(glm::perspective(glm::radians(80.0f), (Globals::gWidth / (float)Globals::gHeight), 0.1f, 1000.0f));
		glm::vec4 eyeCoords = invertedProj * deviceCoord;
		eyeCoords = glm::vec4(eyeCoords.x,eyeCoords.y,-1.0f,0.0f);
		glm::mat4 viewMatrix = Globals::debugCamera.GetView();
		eyeCoords = glm::inverse(viewMatrix) * eyeCoords;
		return glm::normalize(glm::vec3(eyeCoords.x,eyeCoords.y,eyeCoords.z));
	}
}

#endif