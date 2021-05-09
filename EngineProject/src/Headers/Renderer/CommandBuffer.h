#ifndef commandbuffer_h
#define commandbuffer_h

#include	"../../vendor/volk.h"
#include	<iostream>
namespace Engine{
	//����� ��� �������� ������
	class CommandBuffer{
		VkCommandBuffer vCommandBuffer;
		public:
		void AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool){ 
			VkCommandBufferAllocateInfo allocInfo{};
			{
				allocInfo.commandBufferCount = 1;
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = commandPool;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			}

			//�������� ��������� ����� �� ����
			vkAllocateCommandBuffers(device, &allocInfo, &vCommandBuffer);
			
		}

		//������ ������ ������
		void BeginCommandBuffer() { 
			
			VkCommandBufferBeginInfo beginInfo = {};
			{
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			}

			
			if (vkBeginCommandBuffer(vCommandBuffer, &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("Failed to begin command buffer");
			}
		}

		//��������� ����� � �������
		void SubmitCommandBuffer(VkQueue queue){ 
			VkCommandBuffer pCommandBuffers[] = { vCommandBuffer };
			VkSubmitInfo submitInfo{};
			{
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.pCommandBuffers = pCommandBuffers;
				submitInfo.commandBufferCount = 1;
			}

			//��������� ����� � �������
			vkQueueSubmit(queue, 1, &submitInfo, nullptr);
			vkQueueWaitIdle(queue);
		}

		void FreeCommandBuffer(VkDevice device, VkCommandPool commandPool){ //<���������� �����>
		
			if (vCommandBuffer!=VK_NULL_HANDLE)
			{
				vkFreeCommandBuffers(device, commandPool, 1, &vCommandBuffer);
				vCommandBuffer = VK_NULL_HANDLE;
			}
			
		}

		void EndCommandBuffer() {
			if (vkEndCommandBuffer(vCommandBuffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to record command buffer");
			}
			
		}

	public:
		VkCommandBuffer Get() {
			return vCommandBuffer;
		}
	};
}

#endif