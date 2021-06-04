#ifndef commandbuffer_h
#define commandbuffer_h

#include	"../../vendor/volk.h"
#include    "../Globals.h"

#include	<iostream>
namespace Engine {
    //Буфер для хранения команд//
    class CommandBuffer {
        VkCommandBuffer vCommandBuffer;
    public:
        //Выделить память из пула команд//
        void AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool) {
            VkCommandBufferAllocateInfo allocInfo{};
            {
                allocInfo.commandBufferCount = 1;
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = commandPool;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            }

            //Выделить командный буфер из пула
            EngineExec(
                vkAllocateCommandBuffers(device, &allocInfo, &vCommandBuffer),
                "Command Buffer Allocation"
            );
        }

        //Начать запись команд//
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

        //Отправить буфер в очередь//
        void SubmitCommandBuffer(VkQueue queue) {
            VkCommandBuffer pCommandBuffers[] = { vCommandBuffer };
            VkSubmitInfo submitInfo{};
            {
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.pCommandBuffers = pCommandBuffers;
                submitInfo.commandBufferCount = 1;
            }

            //Отправить буфер в очередь
            EngineExec(
                vkQueueSubmit(queue, 1, &submitInfo, nullptr),
                "Submit to Queue"
            );
            vkQueueWaitIdle(queue);
        }

        //Освободить буфер от команд//
        void FreeCommandBuffer(VkDevice device, VkCommandPool commandPool) { //<Освободить буфер>
            if (vCommandBuffer != VK_NULL_HANDLE)
            {
                vkFreeCommandBuffers(device, commandPool, 1, &vCommandBuffer);
                vCommandBuffer = VK_NULL_HANDLE;
            }
        }

        //Закончить запись//
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