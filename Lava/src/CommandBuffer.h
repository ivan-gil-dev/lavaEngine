#pragma once
#include "../vendor/volk.h"
#include <iostream>
namespace Lava{
	class CommandBuffer{
		VkCommandBuffer commandBuffer;
	public:
		void beginCommandBuffer(VkDevice device, VkCommandPool commandPool) {
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.commandBufferCount = 1;
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("Failed to begin command buffer");
			}

		}

		void endSubmitAndFreeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue) {
			if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer");
			}
			VkCommandBuffer pCommandBuffers[] = { commandBuffer };
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pCommandBuffers = pCommandBuffers;
			submitInfo.commandBufferCount = 1;
			vkQueueSubmit(queue, 1, &submitInfo, nullptr);
			vkQueueWaitIdle(queue);
			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
		}

		void freeCommandBuffer(VkDevice device, VkCommandPool commandPool){
			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
		}

		void endCommandBuffer() {
			if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer");
			}
		}

		VkCommandBuffer get() {
			return commandBuffer;
		}

	};
}
