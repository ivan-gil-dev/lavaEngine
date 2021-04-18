#ifndef syncobjects_h
#define syncobjects_h

#include "../../vendor/volk.h"
#include "DataTypes.h"
#include <vector>

namespace Engine{
	class SyncObjects {
		std::vector<VkSemaphore> ImageRenderedSemaphores;
		std::vector<VkSemaphore> ImageAvailableSemaphores;
		std::vector<VkFence> Fences;
	public:
		void CreateSyncObjects(VkDevice device) {
			ImageRenderedSemaphores.resize(Globals::gmax_frames);
			ImageAvailableSemaphores.resize(Globals::gmax_frames);
			Fences.resize(Globals::gmax_frames);
			VkSemaphoreCreateInfo semaphoreCreateInfo{};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VkFenceCreateInfo fenceCreateInfo{};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			for (size_t i = 0; i < Globals::gmax_frames; i++) {

				if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &ImageRenderedSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(device, &fenceCreateInfo, nullptr, &Fences[i]) != VK_SUCCESS) {
					throw std::runtime_error("Failed to create sync object");
				}
			}
		}

		std::vector<VkSemaphore> GetImageRenderedSemaphores() {
			return ImageRenderedSemaphores;
		}

		std::vector<VkSemaphore> GetImageAvailableSemaphores() {
			return ImageAvailableSemaphores;
		}

		std::vector<VkFence> GetFences() {
			return Fences;
		}

		void DestroySyncObjects(VkDevice device) {
			for (size_t i = 0; i < Globals::gmax_frames; i++) {
				vkDestroySemaphore(device, ImageRenderedSemaphores[i], nullptr);
				vkDestroySemaphore(device, ImageAvailableSemaphores[i], nullptr);
				vkDestroyFence(device, Fences[i], nullptr);
			}
		}

	};

	//namespace Globals{
	//	extern SyncObjects gSyncObjects;
	//}
}

#endif