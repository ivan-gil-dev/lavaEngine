#ifndef commandbuffer_h
#define commandbuffer_h

#include	"../../vendor/volk.h"
#include    "../Globals.h"

#include	<iostream>
namespace Engine {
    //����� ��� �������� ������//
    class CommandBuffer {
        VkCommandBuffer vCommandBuffer;
    public:
        //�������� ������ �� ���� ������//
        void AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool) {
            VkCommandBufferAllocateInfo allocInfo{};
            {
                allocInfo.commandBufferCount = 1;
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = commandPool;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            }

            //�������� ��������� ����� �� ����
            EngineExec(
                vkAllocateCommandBuffers(device, &allocInfo, &vCommandBuffer),
                "Command Buffer Allocation"
            );
        }

        //������ ������ ������//
        void BeginCommandBuffer() {
            VkCommandBufferBeginInfo beginInfo = {};
            {
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            }

            EngineExec(
                vkBeginCommandBuffer(vCommandBuffer, &beginInfo),
                "Begin Command Buffer"
            );
        }

        //��������� ����� � �������//
        void SubmitCommandBuffer(VkQueue queue) {
            VkCommandBuffer pCommandBuffers[] = { vCommandBuffer };
            VkSubmitInfo submitInfo{};
            {
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.pCommandBuffers = pCommandBuffers;
                submitInfo.commandBufferCount = 1;
            }

            //��������� ����� � �������
            EngineExec(
                vkQueueSubmit(queue, 1, &submitInfo, nullptr),
                "Submit to Queue"
            );
            vkQueueWaitIdle(queue);
        }

        //���������� ����� �� ������//
        void FreeCommandBuffer(VkDevice device, VkCommandPool commandPool) { //<���������� �����>
            if (vCommandBuffer != VK_NULL_HANDLE)
            {
                vkFreeCommandBuffers(device, commandPool, 1, &vCommandBuffer);
                vCommandBuffer = VK_NULL_HANDLE;
            }
        }

        //��������� ������//
        void EndCommandBuffer() {
            EngineExec(
                vkEndCommandBuffer(vCommandBuffer),
                "End Command Buffer"
            );
        }

    public:
        VkCommandBuffer Get() {
            return vCommandBuffer;
        }
    };
}

#endif