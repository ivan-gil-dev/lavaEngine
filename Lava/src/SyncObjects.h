#pragma once
#include "../vendor/volk.h"
#include "DataTypes.h"
#include <vector>
namespace Lava{
	class SyncObjects {
		std::vector<VkSemaphore> imageRenderedSemaphores;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkFence> fences;
	public:
		void createSyncObjects(VkDevice device) {
			imageRenderedSemaphores.resize(MAX_FRAMES);
			imageAvailableSemaphores.resize(MAX_FRAMES);
			fences.resize(MAX_FRAMES);
			VkSemaphoreCreateInfo semaphoreCreateInfo{};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VkFenceCreateInfo fenceCreateInfo{};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

			for (size_t i = 0; i < MAX_FRAMES; i++) {

				if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageRenderedSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(device, &fenceCreateInfo, nullptr, &fences[i]) != VK_SUCCESS) {
					throw std::runtime_error("Failed to create sync object");
				}
			}
		}
		std::vector<VkSemaphore> getImageRenderedSemaphores() {
			return imageRenderedSemaphores;
		}
		std::vector<VkSemaphore> getImageAvailableSemaphores() {
			return imageAvailableSemaphores;
		}
		std::vector<VkFence> getFences() {
			return fences;
		}
		void destroySyncObjects(VkDevice device) {
			for (size_t i = 0; i < MAX_FRAMES; i++) {
				vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
				vkDestroySemaphore(device, imageRenderedSemaphores[i], nullptr);
				vkDestroyFence(device, fences[i], nullptr);
			}
		}
	};
}
