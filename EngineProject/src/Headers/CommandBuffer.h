#ifndef commandbuffer_h
#define commandbuffer_h

#include	"../../vendor/volk.h"
#include	<iostream>
namespace Engine{
	//<����� ��� ������ ������>
	class CommandBuffer{
		VkCommandBuffer CommandBuffer;
		public:
		void AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool){ 
			VkCommandBufferAllocateInfo allocInfo{};
			{
				allocInfo.commandBufferCount = 1;
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = commandPool;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			}

			//<�������� ��������� ����� �� ����>
			vkAllocateCommandBuffers(device, &allocInfo, &CommandBuffer);
			
		}

		//<������ ������ ������>
		void BeginCommandBuffer() { 
			
			VkCommandBufferBeginInfo beginInfo = {};
			{
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			}

			
			if (vkBeginCommandBuffer(CommandBuffer, &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("Failed to begin command buffer");
			}
		}

		//<��������� ����� � �������>
		void SubmitCommandBuffer(VkQueue queue){ 
			VkCommandBuffer pCommandBuffers[] = { CommandBuffer };
			VkSubmitInfo submitInfo{};
			{
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.pCommandBuffers = pCommandBuffers;
				submitInfo.commandBufferCount = 1;
			}

			//<��������� ����� � �������>
			vkQueueSubmit(queue, 1, &submitInfo, nullptr);
			vkQueueWaitIdle(queue);
		}

		void FreeCommandBuffer(VkDevice device, VkCommandPool commandPool){ //<���������� �����>
			vkFreeCommandBuffers(device, commandPool, 1, &CommandBuffer);
		}

		void EndCommandBuffer() {
			if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer");
			}
		}
	//Getters
	public:
		VkCommandBuffer Get() {
			return CommandBuffer;
		}
	};
}

#endif