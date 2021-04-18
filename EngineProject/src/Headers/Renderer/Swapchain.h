#ifndef swapchain_h
#define swapchain_h

#include	"../../vendor/volk.h"
#include	"DataTypes.h"
#include	<vector>
#include	<iostream>
namespace Engine{
	class Swapchain {
		VkSwapchainKHR swapchain;
		VkSwapchainCreateInfoKHR CreateInfo{};
		std::vector<VkImage> SwapchainImages;
		std::vector<VkImageView> SwapchainImageViews;
		std::vector<VkFramebuffer> SwapchainFramebuffers;
	public:
		void CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface,
			DataTypes::QueueIndices_t indices);
		
		void CreateImageViews(VkDevice device);

		VkSwapchainKHR Get();

		void DestroySwapchainObjects(VkDevice device);

		VkSwapchainCreateInfoKHR GetInfo();

		std::vector<VkImageView>* PGetImageViews();

		std::vector<VkFramebuffer> GetSwapchainFramebuffers();

		void SetFramebuffers(std::vector<VkFramebuffer> framebuffers);

	};

	//namespace Globals{
	//	extern Swapchain gSwapchain;
	//}
	
}

#endif