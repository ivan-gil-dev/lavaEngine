#ifndef physicaldevice_h
#define physicaldevice_h

#include "../../vendor/volk.h"
#include "DataTypes.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

namespace Engine{
	class PhysicalDevice {
		VkPhysicalDevice vPhysicalDevice;
		VkSampleCountFlagBits maxMSAAsamples;
		DataTypes::QueueIndices_t vQueueIndices;
		bool Suitable;
		public:
		VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDeviceProperties &deviceProperties) {

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
		
		VkPhysicalDevice Get() {
			return vPhysicalDevice;
		}
		
		DataTypes::QueueIndices_t GetQueueIndices() {
			return vQueueIndices;
		}

		void PickPhysicalDevice(VkInstance instance) {
			Suitable = false;

			uint32_t deviceCount;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			for (size_t i = 0; i < deviceCount; i++) {

				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

				if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {

					vPhysicalDevice = devices[i];

					maxMSAAsamples = GetMaxUsableSampleCount(deviceProperties);

					if (Globals::gMSAAsamples > maxMSAAsamples) {
						Globals::gMSAAsamples = maxMSAAsamples;
					}

					spdlog::info("MSAA Level {:02d}", Globals::gMSAAsamples);
					break;
				}
			}

			uint32_t propertyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(vPhysicalDevice, &propertyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueProperties(propertyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(vPhysicalDevice, &propertyCount, queueProperties.data());

			for (size_t i = 0; i < propertyCount; i++) {
				if (queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {

					vQueueIndices.graphicsQueueIndex = (uint32_t)i;
					vQueueIndices.presentQueueIndex = (uint32_t)i;
					Suitable = true;
					break;

				}
			}

			if (!Suitable) {
				throw std::runtime_error("GPU that supports Vulkan is not detected");
			}
		}

	};

	namespace Globals{
		extern PhysicalDevice gPhysicalDevice;
	}
}

#endif