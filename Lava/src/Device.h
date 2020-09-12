#pragma once
#include "../vendor/volk.h"
#include "DataTypes.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
namespace Lava{
	static std::vector<const char*> Layers = { "VK_LAYER_KHRONOS_validation" };
	static std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	class Device {
		VkDevice logicalDevice;
		VkDeviceCreateInfo createInfo{};
		VkQueue graphicsQueue;
		public:
		void createDevice(VkPhysicalDevice device, QueueIndices indices) {
			float priority = 1.0f;
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.queueCount = 1;
			queueInfo.queueFamilyIndex = indices.graphicsQueueIndex;
			queueInfo.pQueuePriorities = &priority;
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfos.push_back(queueInfo);

			VkPhysicalDeviceFeatures features{};
			features.samplerAnisotropy = VK_TRUE;
			features.imageCubeArray = VK_TRUE;
			createInfo.pEnabledFeatures = &features;
			createInfo.enabledLayerCount = (uint32_t)Layers.size();
			createInfo.ppEnabledLayerNames = Layers.data();
			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

			if (vkCreateDevice(device, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create logical device");
			}
			vkGetDeviceQueue(logicalDevice, indices.graphicsQueueIndex, 0, &graphicsQueue);
		}

		VkQueue getGraphicsQueue() {
			return graphicsQueue;
		}

		VkDevice get() {
			return logicalDevice;
		}

		void destroy() {
			vkDestroyDevice(logicalDevice, nullptr);
		}

		VkDevice* pGet() {
			return &logicalDevice;
		}

	};

}

