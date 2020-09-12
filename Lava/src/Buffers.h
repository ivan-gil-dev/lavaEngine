#pragma once
#include "../vendor/volk.h"

#include "CommandBuffer.h"
#include <iostream>
namespace Lava{
	uint32_t findMemoryType(VkPhysicalDevice device, uint32_t filter, VkMemoryPropertyFlags flags) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type");
	}

	void CreateBuffer(
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& deviceMemory
	) {
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create vertex buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memRequirements.size;
		memAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(device, &memAllocInfo, nullptr, &deviceMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate memory");
		}

		vkBindBufferMemory(device, buffer, deviceMemory, 0);
	}



	void CopyBuffer(
		VkDevice device,
		VkCommandPool commandPool,
		VkQueue copyBufferQueue,
		VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize srcOffset,
		VkDeviceSize dstOffset,
		VkDeviceSize size
	) {
		CommandBuffer commandBuffer;
		commandBuffer.beginCommandBuffer(device, commandPool);

		VkBufferCopy bufferRegion{};
		bufferRegion.srcOffset = srcOffset;
		bufferRegion.dstOffset = dstOffset;
		bufferRegion.size = size;

		vkCmdCopyBuffer(commandBuffer.get(), srcBuffer, dstBuffer, 1, &bufferRegion);
		commandBuffer.endSubmitAndFreeCommandBuffer(device, commandPool, copyBufferQueue);

	}


	class Buffer {
	protected:
		VkBuffer buffer;
		VkDeviceMemory memory;
	public:
		VkBuffer get() {
			return buffer;
		}

		VkDeviceMemory getSize() {
			return memory;
		}

		void destroy(VkDevice device) {
			vkFreeMemory(device, memory, nullptr);
			vkDestroyBuffer(device, buffer, nullptr);
		}

	};

	class VertexBuffer : public Buffer {
	public:
		void CreateVertexBuffer(
			VkPhysicalDevice physicalDevice, 
			VkDevice device, 
			VkQueue copyBufferQueue, 
			VkCommandPool commandPool, 
			void* bufferData, 
			size_t sizeOfData
		) {

			VkDeviceSize bufferSize = sizeOfData;

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingMemory;

			CreateBuffer(
				physicalDevice,
				device, 
				bufferSize, 
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				stagingBuffer, 
				stagingMemory
			);

			void* data;
			vkMapMemory(device, stagingMemory, 0, bufferSize, 0, &data);
			memcpy(data, bufferData, bufferSize);
			vkUnmapMemory(device, stagingMemory);

			CreateBuffer(
				physicalDevice, 
				device,
				bufferSize, 
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				buffer, 
				memory
			);

			CopyBuffer(
				device, 
				commandPool, 
				copyBufferQueue, 
				stagingBuffer, 
				buffer,
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
		void CreateIndexBuffer(
			VkPhysicalDevice physicalDevice, 
			VkDevice device, 
			VkQueue copyBufferQueue, 
			VkCommandPool commandPool, 
			void* bufferData, 
			size_t sizeOfData
		) {
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingMemory;

			CreateBuffer(
				physicalDevice, 
				device, 
				sizeOfData,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				stagingBuffer, stagingMemory
			);

			void* data;
			vkMapMemory(device, stagingMemory, 0, sizeOfData, 0, &data);
			memcpy(data, bufferData, sizeOfData);
			vkUnmapMemory(device, stagingMemory);

			CreateBuffer(
				physicalDevice, 
				device, 
				sizeOfData,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				buffer, 
				memory
			);
			CopyBuffer(
				device, 
				commandPool, 
				copyBufferQueue, 
				stagingBuffer, 
				buffer,
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
			CreateBuffer(
				physicalDevice, 
				device, 
				uniformBufferSize, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buffer, 
				memory
			);
		}
	};

}

