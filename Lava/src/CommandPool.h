#pragma once
#include "../vendor/volk.h"
#include "DataTypes.h"
namespace Lava{
	class CommandPool {
		VkCommandPool commandPool;
	public:
		void createCommandPool(QueueIndices indices, VkDevice device) {
			VkCommandPoolCreateInfo commandPoolCreateInfo{};
			commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolCreateInfo.queueFamilyIndex = indices.graphicsQueueIndex;
			if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create command pool");
			}
		}
		VkCommandPool get() {
			return commandPool;
		}
		VkCommandPool* pGet() {
			return &commandPool;
		}
		void destroy(VkDevice device) {
			vkDestroyCommandPool(device, commandPool, nullptr);
		}
	};

}
