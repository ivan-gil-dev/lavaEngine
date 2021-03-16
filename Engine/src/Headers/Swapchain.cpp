#include "Swapchain.h"

void Engine::Swapchain::CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, DataTypes::QueueIndices_t indices) {
	//Проверка поддержки swapchain
	VkBool32 swapchainSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, indices.graphicsQueueIndex, surface, &swapchainSupported);

	if (swapchainSupported != VK_TRUE) {
		throw std::runtime_error("Surface is not supported");
	}

	CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

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
			CreateInfo.imageColorSpace = surfaceFormats[i].colorSpace;
			CreateInfo.imageFormat = surfaceFormats[i].format;
			break;
		}
	}

	for (size_t i = 0; i < presentModesCount; i++) {
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			CreateInfo.presentMode = presentModes[i];
			break;
		}
	}

	if (CreateInfo.presentMode != VK_PRESENT_MODE_MAILBOX_KHR) {
		CreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	CreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
	CreateInfo.preTransform = surfaceCapabilities.currentTransform;
	CreateInfo.clipped = VK_FALSE;
	CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	CreateInfo.imageArrayLayers = 1;

	VkExtent2D extent = { surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height };
	spdlog::info("WIDTH DEBUG {:05d}", surfaceCapabilities.currentExtent.width);


	CreateInfo.imageExtent = extent;
	CreateInfo.surface = surface;

	if (vkCreateSwapchainKHR(device, &CreateInfo, nullptr, &swapchain)) {
		throw std::runtime_error("Failed to create swapchain");
	}

	uint32_t imagesCount;
	vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, nullptr);
	SwapchainImages.resize(imagesCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, SwapchainImages.data());
}

void Engine::Swapchain::CreateImageViews(VkDevice device) {
	SwapchainImageViews.resize(SwapchainImages.size());
	for (size_t i = 0; i < SwapchainImages.size(); i++) {
		VkImageViewCreateInfo ImageViewCreateInfo{};
		ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewCreateInfo.format = CreateInfo.imageFormat;
		ImageViewCreateInfo.image = SwapchainImages[i];
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
		if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &SwapchainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Image View");
		}
	}
}

VkSwapchainKHR Engine::Swapchain::Get() {
	return swapchain;
}

void Engine::Swapchain::DestroySwapchainObjects(VkDevice device) {
	for (size_t i = 0; i < SwapchainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(device, SwapchainFramebuffers[i], nullptr);
	}
	for (size_t i = 0; i < SwapchainFramebuffers.size(); i++) {
		vkDestroyImageView(device, SwapchainImageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

VkSwapchainCreateInfoKHR Engine::Swapchain::GetInfo() {
	return CreateInfo;
}

std::vector<VkImageView>* Engine::Swapchain::PGetImageViews() {
	return &SwapchainImageViews;
}

std::vector<VkFramebuffer> Engine::Swapchain::GetSwapchainFramebuffers() {
	return SwapchainFramebuffers;
}

void Engine::Swapchain::SetFramebuffers(std::vector<VkFramebuffer> framebuffers) {
	this->SwapchainFramebuffers = framebuffers;
}

