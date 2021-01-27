#pragma once
#include "../vendor/volk.h"
#include "CommandBuffer.h"
#include <iostream>

namespace Lava{
	namespace VulkanBuffers{
		//Найти доступный тип памяти (память GPU, RAM и т.д.)
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

		//Создание буфера
		void gFunc_CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, 
			VkBuffer& buffer,VkDeviceMemory& deviceMemory, VkBufferUsageFlags usage, 
			VkMemoryPropertyFlags propertyFlags) {

			VkBufferCreateInfo bufferCreateInfo{};
			//Заполнение структуры
			{
				bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				bufferCreateInfo.size = size;
				bufferCreateInfo.usage = usage;
			}
			
			if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create vertex buffer");
			}

			//Получение сведений,
			//необходимых для выделения памяти 
			//в зависимости от флагов назначения буфера
			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements); 

			VkMemoryAllocateInfo memAllocInfo{};
			//Заполнение структуры
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

		//Перемещение данных из буфера в буфер
		void gFunc_CopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue copyBufferQueue,
			VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset,
			VkDeviceSize dstOffset, VkDeviceSize size) {


			CommandBuffer commandBuffer;
			// Начало записи команд
			{
				commandBuffer.AllocateCommandBuffer(device, commandPool);
				commandBuffer.BeginCommandBuffer();
			}
			
			VkBufferCopy bufferRegion{};
			// Заполнение структуры
			{
				bufferRegion.srcOffset = srcOffset;
				bufferRegion.dstOffset = dstOffset;
				bufferRegion.size = size;
			}
			
			// Копирование буфера
			vkCmdCopyBuffer(commandBuffer.Get(), srcBuffer, dstBuffer, 1, &bufferRegion);

			// Конец записи команд
			{
				commandBuffer.EndCommandBuffer();
				commandBuffer.SubmitCommandBuffer(copyBufferQueue);
				commandBuffer.FreeCommandBuffer(device, commandPool);
			}
			
		}

		// Базовый класс буфера (без реализации выделения памяти)
		class Buffer {
		protected:
			//Буфер
			VkBuffer	    mBuffer;
			//Память, которая будет привязана к буферу
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
			//Создание и заполнение вертексного буфера
			void CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue copyBufferQueue,
				VkCommandPool commandPool, void* bufferData, size_t sizeOfData) {
				//Буфер будет создан в памяти хоста потом будет перемещен в GPU
				//Это связано с тем, что данные (а именно вертексы)
				//не будут динамически изменятся в главном цикле,
				//соответственно команда vkMapMemory не будет использоваться
				//для доступа к памяти GPU.
				

				VkDeviceSize bufferSize = sizeOfData;

				// Буфер в RAM
				VkBuffer stagingBuffer;
				VkDeviceMemory stagingMemory;

				// Создание буфера
				gFunc_CreateBuffer(
					physicalDevice,
					device,
					bufferSize,
					stagingBuffer,
					stagingMemory,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);

				// Передача данных в буфер
				void* data;
				vkMapMemory(device, stagingMemory, 0, bufferSize, 0, &data);
				memcpy(data, bufferData, bufferSize);
				vkUnmapMemory(device, stagingMemory);

				// Создание буфера в GPU
				gFunc_CreateBuffer(
					physicalDevice,
					device,
					bufferSize,
					mBuffer,
					Memory,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				);

				// Перемещение данных в GPU
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
			//Создание и заполнение индексного буфера
			void CreateIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue copyBufferQueue,
				VkCommandPool commandPool, void* bufferData, size_t sizeOfData) {
				//Буфер будет создан в памяти хоста потом будет перемещен в GPU
				//Это связано с тем, что данные (а именно индексы)
				//не будут динамически изменятся в главном цикле,
				//соответственно команда vkMapMemory не будет использоваться
				//для доступа к памяти GPU.
				VkBuffer stagingBuffer;
				VkDeviceMemory stagingMemory;

				// Создание буфера
				gFunc_CreateBuffer(
					physicalDevice,
					device,
					sizeOfData,
					stagingBuffer,
					stagingMemory,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);

				// Передача данных в буфер
				void* data;
				vkMapMemory(device, stagingMemory, 0, sizeOfData, 0, &data);
				memcpy(data, bufferData, sizeOfData);
				vkUnmapMemory(device, stagingMemory);

				// Создание буфера в GPU
				gFunc_CreateBuffer(
					physicalDevice,
					device,
					sizeOfData,
					mBuffer,
					Memory,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				);

				// Перемещение данных из ОЗУ в GPU
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
				//Буфер для передачи в Uniform'ы
				//можно создать напрямую в GPU
			
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

