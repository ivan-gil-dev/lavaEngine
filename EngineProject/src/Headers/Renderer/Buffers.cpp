#include "Buffers.h"

VkBuffer Engine::VulkanBuffers::Buffer::Get() {
	return mBuffer;
}

VkDeviceMemory Engine::VulkanBuffers::Buffer::GetDeviceMemory() {
	return Memory;
}

void Engine::VulkanBuffers::Buffer::Destroy(VkDevice device) {
	vkFreeMemory(device, Memory, nullptr);
	vkDestroyBuffer(device, mBuffer, nullptr);
}

//Вершинный буфер
void Engine::VulkanBuffers::VertexBuffer::CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, 
	VkQueue copyBufferQueue, VkCommandPool commandPool, void* bufferData, size_t sizeOfData) {

	VkDeviceSize bufferSize = sizeOfData;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	//Создание буфера в памяти хоста
	//Буфер доступен для записи
	Buf_Func_CreateBuffer(
		physicalDevice,
		device,
		bufferSize,
		stagingBuffer,
		stagingMemory,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	//Запись данных в буфер
	void* data;
	vkMapMemory(device, stagingMemory, 0, bufferSize, 0, &data);
	memcpy(data, bufferData, bufferSize);
	vkUnmapMemory(device, stagingMemory);

	//Создание буфера в памяти GPU недоступного для записи из хоста
	Buf_Func_CreateBuffer(
		physicalDevice,
		device,
		bufferSize,
		mBuffer,
		Memory,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	//Перемещение данных из буфера в памяти хоста
	//в буфер в памяти GPU
	Buf_Func_CopyBuffer(
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

//Буфер с индексами
void Engine::VulkanBuffers::IndexBuffer::CreateIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue copyBufferQueue, 
	VkCommandPool commandPool, void* bufferData, size_t sizeOfData) {

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	Buf_Func_CreateBuffer(
		physicalDevice,
		device,
		sizeOfData,
		stagingBuffer,
		stagingMemory,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* data;
	vkMapMemory(device, stagingMemory, 0, sizeOfData, 0, &data);
	memcpy(data, bufferData, sizeOfData);
	vkUnmapMemory(device, stagingMemory);


	Buf_Func_CreateBuffer(
		physicalDevice,
		device,
		sizeOfData,
		mBuffer,
		Memory,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	Buf_Func_CopyBuffer(
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

//Буфер с данными для передачи в пискельный шейдер
void Engine::VulkanBuffers::UniformBuffer::CreateUniformBuffer(VkPhysicalDevice physicalDevice, VkDevice device, size_t size) {
	VkDeviceSize uniformBufferSize = size;
	
	//Буфер для пиксельного шейдера создается в памяти хоста
	Buf_Func_CreateBuffer(
		physicalDevice,
		device,
		uniformBufferSize,
		mBuffer,
		Memory,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}
