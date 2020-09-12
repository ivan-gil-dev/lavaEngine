#pragma once
#include "../vendor/volk.h"
#include "DataTypes.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
namespace Lava{
	class PhysicalDevice {
		VkPhysicalDevice physicalDevice;
		QueueIndices queueIndices;
		bool Suitable;
	public:
		VkSampleCountFlagBits getMaxUsableSampleCount() {
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

			VkSampleCountFlags counts = deviceProperties.limits.framebufferColorSampleCounts &
				deviceProperties.limits.framebufferDepthSampleCounts;
			if (counts & VK_SAMPLE_COUNT_64_BIT) {
				return VK_SAMPLE_COUNT_64_BIT;
			}
			if (counts & VK_SAMPLE_COUNT_32_BIT) {
				return VK_SAMPLE_COUNT_32_BIT;
			}
			if (counts & VK_SAMPLE_COUNT_16_BIT) {
				return VK_SAMPLE_COUNT_16_BIT;
			}
			if (counts & VK_SAMPLE_COUNT_8_BIT) {
				return VK_SAMPLE_COUNT_8_BIT;
			}
			if (counts & VK_SAMPLE_COUNT_4_BIT) {
				return VK_SAMPLE_COUNT_4_BIT;
			}
			if (counts & VK_SAMPLE_COUNT_2_BIT) {
				return VK_SAMPLE_COUNT_2_BIT;
			}
			return VK_SAMPLE_COUNT_1_BIT;

		}
		
		VkPhysicalDevice get() {
			return physicalDevice;
		}
		
		QueueIndices getQueueIndices() {
			return queueIndices;
		}


		void pickPhysicalDevice(VkInstance instance) {
			Suitable = false;

			uint32_t deviceCount;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			for (size_t i = 0; i < deviceCount; i++) {
				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
				if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
					physicalDevice = devices[i];
					maxMSAAsamples = getMaxUsableSampleCount();
					spdlog::info("MSAA Level {:02d}",MSAAsamples);
					break;
				}
			}

			uint32_t propertyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueProperties(propertyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertyCount, queueProperties.data());
			for (size_t i = 0; i < propertyCount; i++) {
				if (queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					queueIndices.graphicsQueueIndex = (uint32_t)i;
					queueIndices.presentQueueIndex = (uint32_t)i;
					Suitable = true;
					break;
				}
			}

			if (!Suitable) {
				throw std::runtime_error("No Suitable Physical Devices");
			}
		}
	};
}
