#pragma once
#include	"../vendor/volk.h"
#include	"DataTypes.h"
#include	<GLFW/glfw3.h>
#include	<vector>
#include	<iostream>
namespace Lava{
	static std::vector<const char*> Layers = { "VK_LAYER_KHRONOS_validation" };
	static std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	class Device {
		public:
		VkDevice LogicalDevice;
		VkDeviceCreateInfo CreateInfo{};
		VkQueue GraphicsQueue;
		public:
		void CreateDevice(VkPhysicalDevice device, DataTypes::QueueIndices indices) {
			float priority = 1.0f;

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.queueCount = 1;
			queueInfo.queueFamilyIndex = indices.graphicsQueueIndex; //Индекс очереди, поддерживающий команды отрисовки
			queueInfo.pQueuePriorities = &priority;
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfos.push_back(queueInfo);

			VkPhysicalDeviceFeatures features{};
			features.samplerAnisotropy = VK_TRUE;
			features.imageCubeArray = VK_TRUE;
			features.fillModeNonSolid = VK_TRUE;

			CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			CreateInfo.pEnabledFeatures = &features;
			CreateInfo.enabledLayerCount = (uint32_t)Layers.size();
			CreateInfo.ppEnabledLayerNames = Layers.data();
			CreateInfo.pQueueCreateInfos = queueCreateInfos.data();
			CreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
			CreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			CreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

			if (vkCreateDevice(device, &CreateInfo, nullptr, &LogicalDevice) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create logical device");
			}
			vkGetDeviceQueue(LogicalDevice, indices.graphicsQueueIndex, 0, &GraphicsQueue);
		}

		VkQueue GetGraphicsQueue() {
			return GraphicsQueue;
		}

		VkDevice Get() {
			return LogicalDevice;
		}

		void Destroy() {
			vkDestroyDevice(LogicalDevice, nullptr);
		}

		VkDevice* PGet() {
			return &LogicalDevice;
		}
	
	}gDevice;

}

