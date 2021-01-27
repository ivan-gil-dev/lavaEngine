#pragma once
#include	"../vendor/volk.h"
#include	"DataTypes.h"
#include	<vector>
#include	<iostream>
namespace Lava{
	class Swapchain {
		VkSwapchainKHR swapchain;
		VkSwapchainCreateInfoKHR createInfo{};
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		std::vector<VkFramebuffer> swapchainFramebuffers;
	public:
		void createSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, QueueIndices indices) {
			//Проверка поддержки swapchain
			VkBool32 swapchainSupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, indices.graphicsQueueIndex, surface, &swapchainSupported);

			if (swapchainSupported != VK_TRUE) {
				throw std::runtime_error("Surface is not supported");
			}

			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

			VkSurfaceCapabilitiesKHR surfaceCapabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

			uint32_t presentModesCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);
			std::vector<VkPresentModeKHR> presentModes(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, presentModes.data());

			uint32_t surfaceFormatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
			std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

			for (size_t i = 0; i < surfaceFormatCount; i++) {
				if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
					surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					createInfo.imageColorSpace = surfaceFormats[i].colorSpace;
					createInfo.imageFormat = surfaceFormats[i].format;
					break;
				}
			}

			for (size_t i = 0; i < presentModesCount; i++) {
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
					createInfo.presentMode = presentModes[i];
					break;
				}
			}

			if (createInfo.presentMode != VK_PRESENT_MODE_MAILBOX_KHR) {
				createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
			}

			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
			createInfo.preTransform = surfaceCapabilities.currentTransform;
			createInfo.clipped = VK_TRUE;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.imageArrayLayers = 1;
			
			VkExtent2D extent = { WIDTH,HEIGHT };
			createInfo.imageExtent = extent;
			createInfo.surface = surface;

			if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain)) {
				throw std::runtime_error("Failed to create swapchain");
			}

			uint32_t imagesCount;
			vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, nullptr);
			swapchainImages.resize(imagesCount);
			vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, swapchainImages.data());
		}

		void createImageViews(VkDevice device) {
			swapchainImageViews.resize(swapchainImages.size());
			for (size_t i = 0; i < swapchainImages.size(); i++) {
				VkImageViewCreateInfo ImageViewCreateInfo{};
				ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				ImageViewCreateInfo.format = createInfo.imageFormat;
				ImageViewCreateInfo.image = swapchainImages[i];
				ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
				ImageViewCreateInfo.subresourceRange.levelCount = 1;
				ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
				ImageViewCreateInfo.subresourceRange.layerCount = 1;
				ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
					throw std::runtime_error("Failed to create Image View");
				}
			}
		}

		VkSwapchainKHR get() {
			return swapchain;
		}

		void destroySwapchainObjects(VkDevice device) {
			for (size_t i = 0; i < swapchainFramebuffers.size(); i++) {
				vkDestroyFramebuffer(device, swapchainFramebuffers[i], nullptr);
			}
			for (size_t i = 0; i < swapchainImageViews.size(); i++) {
				vkDestroyImageView(device, swapchainImageViews[i], nullptr);

			}
			vkDestroySwapchainKHR(device, swapchain, nullptr);
		}

		VkSwapchainCreateInfoKHR getInfo() {
			return createInfo;
		}

		std::vector<VkImageView>* pGetImageViews() {
			return &swapchainImageViews;
		}

		std::vector<VkFramebuffer> getSwapchainFramebuffers() {
			return swapchainFramebuffers;
		}

		void setFramebuffers(std::vector<VkFramebuffer> framebuffers) {
			this->swapchainFramebuffers = framebuffers;
		}

	}swapchain;
}
