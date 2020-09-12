#pragma once
#include "../vendor/volk.h"
#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#define DEBUG
#ifdef DEBUG
bool enableValidationLayer = true;
#else
bool enableValidationLayer = false;
#endif
namespace Lava{

	static std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		spdlog::warn(pCallbackData->pMessage);
		//std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}


	class Instance {
		VkInstance instance;
		VkApplicationInfo appInfo{};
		VkInstanceCreateInfo createInfo{};
		VkDebugUtilsMessengerEXT debugMessenger;
		VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
	public:
		void createInstance() {

			if (volkInitialize() != VK_SUCCESS) {
				throw std::runtime_error("Failed to init volk");
			}

			appInfo.apiVersion = VK_MAKE_VERSION(1, 2, 0);
			appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
			appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
			appInfo.pApplicationName = "Lava Engine";
			appInfo.pEngineName = "Lava";
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

			uint32_t count;
			const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

			std::vector<const char*> extensions;
			extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + count);
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			createInfo.enabledExtensionCount = uint32_t(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

			if (enableValidationLayer) {
				createInfo.enabledLayerCount = (uint32_t)layers.size();
				createInfo.ppEnabledLayerNames = layers.data();
			}


			messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			messengerInfo.pfnUserCallback = debugCallback;
			messengerInfo.pUserData = nullptr;
			messengerInfo.flags = 0;

			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengerInfo;

			createInfo.pApplicationInfo = &appInfo;
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

			if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create instance");
			}
			volkLoadInstance(instance);

			if (vkCreateDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create debug messenger");
			}


		}

		VkInstance get() {
			return instance;
		}

		void destroy() {
			vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
			vkDestroyInstance(instance, nullptr);
		};

	};
}
