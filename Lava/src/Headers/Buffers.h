#pragma once
#include "../vendor/volk.h"
#include "CommandBuffer.h"
#include <iostream>

namespace Lava{
	namespace VulkanBuffers{
		//����� ��������� ��� ������ (������ GPU, RAM � �.�.)
		uint32_t gFunc_FindMemoryType(VkPhysicalDevice device, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags) {
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				if ((memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags) {
					return i;
				}
			}
			throw std::runtime_error("Unable to find suitable memory type");
		}

		//�������� ������
		void gFunc_CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, 
			VkBuffer& buffer,VkDeviceMemory& deviceMemory, VkBufferUsageFlags usage, 
			VkMemoryPropertyFlags propertyFlags) {

			VkBufferCreateInfo bufferCreateInfo{};
			//���������� ���������
			{
				bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				bufferCreateInfo.size = size;
				bufferCreateInfo.usage = usage;
			}
			
			if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create vertex buffer");
			}

			//��������� ��������,
			//����������� ��� ��������� ������ 
			//� ����������� �� ������ ���������� ������
			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements); 

			VkMemoryAllocateInfo memAllocInfo{};
			//���������� ���������
			{
				memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memAllocInfo.allocationSize = memRequirements.size;
				memAllocInfo.memoryTypeIndex = gFunc_FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, propertyFlags);
			}

			if (vkAllocateMemory(device, &memAllocInfo, nullptr, &deviceMemory) != VK_SUCCESS) {
				throw std::runtime_error("Failed to allocate memory");
			}

			vkBindBufferMemory(device, buffer, deviceMemory, 0);
		}

		//����������� ������ �� ������ � �����
		void gFunc_CopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue copyBufferQueue,
			VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset,
			VkDeviceSize dstOffset, VkDeviceSize size) {


			CommandBuffer commandBuffer;
			// ������ ������ ������
			{
				commandBuffer.AllocateCommandBuffer(device, commandPool);
				commandBuffer.BeginCommandBuffer();
			}
			
			VkBufferCopy bufferRegion{};
			// ���������� ���������
			{
				bufferRegion.srcOffset = srcOffset;
				bufferRegion.dstOffset = dstOffset;
				bufferRegion.size = size;
			}
			
			// ����������� ������
			vkCmdCopyBuffer(commandBuffer.Get(), srcBuffer, dstBuffer, 1, &bufferRegion);

			// ����� ������ ������
			{
				commandBuffer.EndCommandBuffer();
				commandBuffer.SubmitCommandBuffer(copyBufferQueue);
				commandBuffer.FreeCommandBuffer(device, commandPool);
			}
			
		}

		// ������� ����� ������ (��� ���������� ��������� ������)
		class Buffer {
		protected:
			//�����
			VkBuffer	    mBuffer;
			//������, ������� ����� ��������� � ������
			VkDeviceMemory  Memory;
		public:
			VkBuffer Get() {
				return mBuffer;
			}

			VkDeviceMemory GetDeviceMemory() {
				return Memory;
			}

			void Destroy(VkDevice device) {
				vkFreeMemory(device, Memory, nullptr);
				vkDestroyBuffer(device, mBuffer, nullptr);
			}
		};

		class VertexBuffer : public Buffer {
		public:
			//�������� � ���������� ����������� ������
			void CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue copyBufferQueue,
				VkCommandPool commandPool, void* bufferData, size_t sizeOfData) {
				//����� ����� ������ � ������ ����� ����� ����� ��������� � GPU
				//��� ������� � ���, ��� ������ (� ������ ��������)
				//�� ����� ����������� ��������� � ������� �����,
				//�������������� ������� vkMapMemory �� ����� ��������������
				//��� ������� � ������ GPU.
				

				VkDeviceSize bufferSize = sizeOfData;

				// ����� � RAM
				VkBuffer stagingBuffer;
				VkDeviceMemory stagingMemory;

				// �������� ������
				gFunc_CreateBuffer(
					physicalDevice,
					device,
					bufferSize,
					stagingBuffer,
					stagingMemory,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);

				// �������� ������ � �����
				void* data;
				vkMapMemory(device, stagingMemory, 0, bufferSize, 0, &data);
				memcpy(data, bufferData, bufferSize);
				vkUnmapMemory(device, stagingMemory);

				// �������� ������ � GPU
				gFunc_CreateBuffer(
					physicalDevice,
					device,
					bufferSize,
					mBuffer,
					Memory,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				);

				// ����������� ������ � GPU
				gFunc_CopyBuffer(
					device,
					commandPool,
					copyBufferQueue,
					stagingBuffer,
					mBuffer,
					0,
					0,
					bufferSize
				);

				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingMemory, nullptr);
			}
		};

		class IndexBuffer : public Buffer {
		public:
			//�������� � ���������� ���������� ������
			void CreateIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue copyBufferQueue,
				VkCommandPool commandPool, void* bufferData, size_t sizeOfData) {
				//����� ����� ������ � ������ ����� ����� ����� ��������� � GPU
				//��� ������� � ���, ��� ������ (� ������ �������)
				//�� ����� ����������� ��������� � ������� �����,
				//�������������� ������� vkMapMemory �� ����� ��������������
				//��� ������� � ������ GPU.
				VkBuffer stagingBuffer;
				VkDeviceMemory stagingMemory;

				// �������� ������
				gFunc_CreateBuffer(
					physicalDevice,
					device,
					sizeOfData,
					stagingBuffer,
					stagingMemory,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);

				// �������� ������ � �����
				void* data;
				vkMapMemory(device, stagingMemory, 0, sizeOfData, 0, &data);
				memcpy(data, bufferData, sizeOfData);
				vkUnmapMemory(device, stagingMemory);

				// �������� ������ � GPU
				gFunc_CreateBuffer(
					physicalDevice,
					device,
					sizeOfData,
					mBuffer,
					Memory,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				);

				// ����������� ������ �� ��� � GPU
				gFunc_CopyBuffer(
					device,
					commandPool,
					copyBufferQueue,
					stagingBuffer,
					mBuffer,
					0,
					0,
					sizeOfData
				);

				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingMemory, nullptr);

			}
		};

		class UniformBuffer : public Buffer {
		public:
			void CreateUniformBuffer(VkPhysicalDevice physicalDevice, VkDevice device, size_t size) {
				VkDeviceSize uniformBufferSize = size;
				//����� ��� �������� � Uniform'�
				//����� ������� �������� � GPU
			
				gFunc_CreateBuffer(
					physicalDevice,
					device,
					uniformBufferSize,
					mBuffer,
					Memory,
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);
			}
		};
	}
}

