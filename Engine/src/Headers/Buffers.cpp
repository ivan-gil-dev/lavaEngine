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

//_Вершинный буфер
void Engine::VulkanBuffers::VertexBuffer::CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, 
	VkQueue copyBufferQueue, VkCommandPool commandPool, void* bufferData, size_t sizeOfData) {

	VkDeviceSize bufferSize = sizeOfData;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	//_Создание буфера в памяти хоста
	//_доступного для записи
	Buf_Func_CreateBuffer(
		physicalDevice,
		device,
		bufferSize,
		stagingBuffer,
		stagingMemory,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	//_Запись данных в буфер
	void* data;
	vkMapMemory(device, stagingMemory, 0, bufferSize, 0, &data);
	memcpy(data, bufferData, bufferSize);
	vkUnmapMemory(device, stagingMemory);

	//_Создание буфера в памяти GPU недоступного для записи из хоста
	Buf_Func_CreateBuffer(
		physicalDevice,
		device,
		bufferSize,
		mBuffer,
		Memory,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	//_Перемещение данных из буфера в памяти хоста
	//_в буфер в памяти GPU
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

//_Буфер с индексами
void Engine::VulkanBuffers::IndexBuffer::CreateIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, 
	VkQueue copyBufferQueue, VkCommandPool commandPool, void* bufferData, size_t sizeOfData) {

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

//_Буфер с данными для передачи в пискельный шейдер
void Engine::VulkanBuffers::UniformBuffer::CreateUniformBuffer(VkPhysicalDevice physicalDevice, VkDevice device, size_t size) {
	VkDeviceSize uniformBufferSize = size;
	
	//_Буфер для пиксельного шейдера создается в памяти хоста
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
