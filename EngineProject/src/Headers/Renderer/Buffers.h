#ifndef buffers_h
#define buffers_h

#include "../../vendor/volk.h"
#include <spdlog/spdlog.h>
#include "CommandBuffer.h"
#include <iostream>

namespace Engine {
    namespace VulkanBuffers {
        //����� ����������� (�� �������) ���� ������//
        inline uint32_t Buf_Func_FindSuitableMemoryType(VkPhysicalDevice device, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(device, &memProperties);//�������� �������������� ������ GPU

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags) {
                    return i;//����� ������� ������ � ����������� �� �������
                }
            }

            /*
              * VkMemoryPropertyFlags
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT = 0x00000001 - ��������� ������ ���������� (���������� ��� ������)
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT = 0x00000002 - ������� ������ ��������� (������� ���������)
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004 - ������ ��������� ��������� ��� ������ (������� ���������)
            */

            throw std::runtime_error("Unable to find suitable memory type");
        }

        //�������� ������//
        inline void Buf_Func_CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size,
            VkBuffer& buffer, VkDeviceMemory& deviceMemory, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags) {
            VkBufferCreateInfo bufferCreateInfo{};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferCreateInfo.size = size; //������ �����
            bufferCreateInfo.usage = usage; //��� ������

            EngineExec(
                vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer),
                "Buffer Creation"
            );

            /*
                ����� �������� ������ ����� �������� ������
                � ��������� ������ � ������
            */

            //�������� ���������� ��� ��������� ������//
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            VkMemoryAllocateInfo memAllocInfo{};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memAllocInfo.allocationSize = memRequirements.size;//������ � ������
            memAllocInfo.memoryTypeIndex = Buf_Func_FindSuitableMemoryType(physicalDevice, memRequirements.memoryTypeBits, propertyFlags);

            //�������� ������//
            EngineExec(
                vkAllocateMemory(device, &memAllocInfo, nullptr, &deviceMemory),
                "Buffer Memory Allocation"
            );

            //��������� ������//
            vkBindBufferMemory(device, buffer, deviceMemory, 0);
        }

        //����������� �� ������ � �����//
        inline void Buf_Func_CopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue copyBufferQueue,
            VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size) {
            CommandBuffer commandBuffer;
            commandBuffer.AllocateCommandBuffer(device, commandPool);
            //������ ������ ������//
            commandBuffer.BeginCommandBuffer();

            {
                //������� ����������� ������//
                VkBufferCopy bufferRegion{};
                {
                    bufferRegion.srcOffset = srcOffset;
                    bufferRegion.dstOffset = dstOffset;
                    bufferRegion.size = size;
                }

                //����������� ������//
                vkCmdCopyBuffer(commandBuffer.Get(), srcBuffer, dstBuffer, 1, &bufferRegion);
            }

            //����� ������ ������//
            commandBuffer.EndCommandBuffer();
            //��������� ������� � GPU//
            commandBuffer.SubmitCommandBuffer(copyBufferQueue);
            //���������� ������� ������//
            commandBuffer.FreeCommandBuffer(device, commandPool);
        }

        class Buffer {
        protected:
            VkBuffer	    mBuffer;
            VkDeviceMemory  Memory;
        public:
            VkBuffer Get();

            VkDeviceMemory GetDeviceMemory();

            void Destroy(VkDevice device);
        };

        //��������� �����//
        class VertexBuffer : public Buffer {
        public:
            void CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue copyBufferQueue,
                VkCommandPool commandPool, void* bufferData, size_t sizeOfData);
        };

        //����� ��������//
        class IndexBuffer : public Buffer {
        public:
            void CreateIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue copyBufferQueue,
                VkCommandPool commandPool, void* bufferData, size_t sizeOfData);
        };

        //����� ��������//
        class UniformBuffer : public Buffer {
        public:
            void CreateUniformBuffer(VkPhysicalDevice physicalDevice, VkDevice device, size_t size);
        };
    }
}

#endif // 