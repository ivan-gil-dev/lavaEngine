#pragma once
#include	"../vendor/volk.h"
#include	"DataTypes.h"
namespace Lava{
	/*
		��� ��� ������ ������������ ��� ��������� ������ ��� ��������� ������
	*/
	class CommandPool {
		VkCommandPool CommandPool; 
		public :
		void CreateCommandPool(DataTypes::QueueIndices indices, VkDevice device) {// ������� ��� ��� ������
			VkCommandPoolCreateInfo commandPoolCreateInfo{};
			commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolCreateInfo.queueFamilyIndex = indices.graphicsQueueIndex;
			if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &CommandPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create command pool");
			}
		}

		VkCommandPool Get() {
			return CommandPool;
		}

		VkCommandPool* PGet() {
			return &CommandPool;
		}

		void Destroy( VkDevice device) {
			vkDestroyCommandPool(device, CommandPool, nullptr);
		}

	}gCommandPool;

}
