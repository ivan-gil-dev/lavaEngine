#include "Images.h"


void Engine::Img_Func_CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, VkImage& image, 
	VkDeviceMemory& imageTextureMemory, uint32_t width, uint32_t height, VkImageTiling tiling, 
	VkMemoryPropertyFlags properties, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, 
	uint32_t arrayLayers, VkImageCreateFlags flags) {
	VkExtent3D extent = { width,height,1 };

	VkImageCreateInfo imageCreateInfo = {};
	{
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = flags;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = extent;
		imageCreateInfo.mipLevels = (uint32_t)1;
		imageCreateInfo.arrayLayers = arrayLayers;
		imageCreateInfo.samples = samples;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.usage = usage;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = (uint32_t)0;
		imageCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	

	if (vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Image");
	}
	VkMemoryRequirements memRequirements;
	

	VkMemoryAllocateInfo allocateInfo{};

	{
		vkGetImageMemoryRequirements(device, image, &memRequirements);
		allocateInfo.allocationSize = memRequirements.size;
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.memoryTypeIndex = VulkanBuffers::Buf_Func_FindSuitableMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
	}

	

	VkResult result = vkAllocateMemory(device, &allocateInfo, nullptr, &imageTextureMemory);
	if (result != VK_SUCCESS) {
		std::cout << "Error code " << result << std::endl;
		throw std::runtime_error("Failed to allocate image memory");
	}
	vkBindImageMemory(device, image, imageTextureMemory, 0);
}

void Engine::Img_Func_TransitionImageLayout(VkDevice device, VkQueue pipelineBarrierQueue, 
	VkCommandPool commandPool, VkImage image, VkImageLayout oldLayout, 
	VkImageLayout newLayout, VkImageSubresourceRange subresourceRange) {
	CommandBuffer commandBuffer;
	commandBuffer.AllocateCommandBuffer(device, commandPool);
	commandBuffer.BeginCommandBuffer();

	VkImageMemoryBarrier
		memoryBarrier{};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	memoryBarrier.oldLayout = oldLayout;
	memoryBarrier.newLayout = newLayout;
	memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	memoryBarrier.image = image;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags dstStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		memoryBarrier.srcAccessMask = 0;
		memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		memoryBarrier.srcAccessMask = 0;
		memoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}

	memoryBarrier.subresourceRange = subresourceRange;



	vkCmdPipelineBarrier(commandBuffer.Get(),
		sourceStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &memoryBarrier
	);
	commandBuffer.EndCommandBuffer();
	commandBuffer.SubmitCommandBuffer(pipelineBarrierQueue);
	commandBuffer.FreeCommandBuffer(device, commandPool);
}

void Engine::Img_Func_CopyBufferToImage(VkDevice device, VkQueue copyBufferQueue, 
	VkCommandPool commandPool, VkImage image, VkBuffer buffer, uint32_t width, uint32_t height) {
	CommandBuffer commandBuffer;
	commandBuffer.AllocateCommandBuffer(device, commandPool);
	commandBuffer.BeginCommandBuffer();

	VkBufferImageCopy bufferImageCopy{};
	bufferImageCopy.imageExtent.width = width;
	bufferImageCopy.imageExtent.height = height;
	bufferImageCopy.imageExtent.depth = 1;
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0;
	bufferImageCopy.bufferImageHeight = 0;
	bufferImageCopy.imageOffset = { 0,0,0 };

	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.baseArrayLayer = 0;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageSubresource.mipLevel = 0;

	vkCmdCopyBufferToImage(commandBuffer.Get(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	commandBuffer.EndCommandBuffer();
	commandBuffer.SubmitCommandBuffer(copyBufferQueue);
	commandBuffer.FreeCommandBuffer(device, commandPool);
}

void Engine::Img_func_CopyBufferToCubemap(VkDevice device, VkQueue copyBufferQueue, VkCommandPool commandPool,
	VkImage image, VkBuffer buffer, uint32_t width, uint32_t height) {

	CommandBuffer commandBuffer;
	commandBuffer.AllocateCommandBuffer(device, commandPool);
	commandBuffer.BeginCommandBuffer();

	std::vector<VkBufferImageCopy> bufferImageCopyVector(6);

	for (uint32_t i = 0; i < bufferImageCopyVector.size(); i++) {
		VkBufferImageCopy bufferImageCopy{};
		bufferImageCopy.imageExtent.width = width;
		bufferImageCopy.imageExtent.height = height;
		bufferImageCopy.imageExtent.depth = 1;
		bufferImageCopy.bufferOffset = i * width * height * 4;
		bufferImageCopy.bufferRowLength = 0;
		bufferImageCopy.bufferImageHeight = 0;
		bufferImageCopy.imageOffset = { 0,0,0 };

		bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferImageCopy.imageSubresource.baseArrayLayer = i;
		bufferImageCopy.imageSubresource.layerCount = 1;
		bufferImageCopy.imageSubresource.mipLevel = 0;

		bufferImageCopyVector[i] = bufferImageCopy;
	}

	vkCmdCopyBufferToImage(
		commandBuffer.Get(),
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		(uint32_t)bufferImageCopyVector.size(),
		bufferImageCopyVector.data()
	);

	commandBuffer.EndCommandBuffer();
	commandBuffer.SubmitCommandBuffer(copyBufferQueue);
	commandBuffer.FreeCommandBuffer(device, commandPool);
}

void Engine::DepthImage::CreateDepthImageView(VkDevice device) {
	VkImageViewCreateInfo ImageViewCreateInfo{};
	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.format = DepthFormat;
	ImageViewCreateInfo.image = vDepthImage;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 1;
	ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &DepthImageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Image View");
	}
}

void Engine::DepthImage::CreateDepthBuffer(VkDevice logicalDevice, VkQueue commandBufferQueue, 
	VkExtent2D swapchainExtent, VkPhysicalDevice physicalDevice, VkCommandPool commandPool) {
	DepthFormat = VK_FORMAT_D32_SFLOAT;

	Img_Func_CreateImage(
		physicalDevice,
		logicalDevice,
		vDepthImage,
		DepthImageMemory,
		swapchainExtent.width,
		swapchainExtent.height,
		VK_IMAGE_TILING_OPTIMAL,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		DepthFormat,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		Globals::gMSAAsamples,
		(uint32_t)1,
		VK_NULL_HANDLE
	);

	CreateDepthImageView(logicalDevice);

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.layerCount = 1;
	subresourceRange.levelCount = 1;

	Img_Func_TransitionImageLayout(
		logicalDevice,
		commandBufferQueue,
		commandPool,
		vDepthImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		subresourceRange
	);
}

void Engine::DepthImage::Destroy(VkDevice device) {
	vkDestroyImage(device, vDepthImage, nullptr);
	vkFreeMemory(device, DepthImageMemory, nullptr);
	vkDestroyImageView(device, DepthImageView, nullptr);
}

VkFormat Engine::DepthImage::GetDepthFormat() {
	return DepthFormat;
}

VkImage Engine::DepthImage::GetDepthImage() {
	return vDepthImage;
}

VkImageView Engine::DepthImage::GetImageView() {
	return DepthImageView;
}

void Engine::Texture::CreateImageView(VkDevice device) {
	VkImageViewCreateInfo ImageViewCreateInfo{};
	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	ImageViewCreateInfo.image = Image;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 1;
	ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &ImageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Image View");
	}
}

void Engine::Texture::CreateImageSampler(VkDevice device) {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &ImageSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

VkSampler Engine::Texture::GetImageSampler() {
	return ImageSampler;
}

VkImageView Engine::Texture::GetImageView() {
	return ImageView;
}

void Engine::Texture::DestroyImage(VkDevice device) {
	vkDestroySampler(device, ImageSampler, nullptr);
	vkDestroyImage(device, Image, nullptr);
	vkDestroyImageView(device, ImageView, nullptr);
	vkFreeMemory(device, ImageTextureMemory, nullptr);
}

void Engine::Texture::CreateTexture(VkPhysicalDevice physicalDevice, VkDevice device, 
	VkQueue commandBufferQueue, VkCommandPool commandPool, std::string path) {
	int texWidth, texHeight, texChannels;
	Pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!Pixels) {
		Pixels = stbi_load("CoreAssets/decoyTexture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	}

	VkDeviceSize imageSize = texWidth * texHeight * 4;
	VkBuffer stagingBuffer;
	VkDeviceMemory bufferMemory;

	VulkanBuffers::Buf_Func_CreateBuffer(
		physicalDevice,
		device,
		imageSize,
		stagingBuffer,
		bufferMemory,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	);

	void* data;
	vkMapMemory(device, bufferMemory, 0, imageSize, 0, &data);
	memcpy(data, Pixels, (size_t)imageSize);
	vkUnmapMemory(device, bufferMemory);

	stbi_image_free(Pixels);

	Img_Func_CreateImage(
		physicalDevice,
		device,
		Image,
		ImageTextureMemory,
		texWidth,
		texHeight,
		VK_IMAGE_TILING_OPTIMAL,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		(uint32_t)1,
		VK_NULL_HANDLE
	);

	VkImageSubresourceRange subresourceRange;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.layerCount = 1;
	subresourceRange.levelCount = 1;

	Img_Func_TransitionImageLayout(
		device,
		commandBufferQueue,
		commandPool,
		Image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange
	);

	Img_Func_CopyBufferToImage(
		device,
		commandBufferQueue,
		commandPool,
		Image,
		stagingBuffer,
		texWidth,
		texHeight
	);

	Img_Func_TransitionImageLayout(
		device,
		commandBufferQueue,
		commandPool,
		Image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		subresourceRange
	);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, bufferMemory, nullptr);

	CreateImageView(device);
	CreateImageSampler(device);
}

void Engine::Texture::DestroyTexture(VkDevice device) {
	DestroyImage(device);
}

void Engine::MultisamplingBuffer::CreateImageView(VkDevice device, VkFormat swapchainImageFormat) {
	VkImageViewCreateInfo ImageViewCreateInfo{};
	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.format = swapchainImageFormat;
	ImageViewCreateInfo.image = Image;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 1;
	ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &ImageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Image View");
	}
}

void Engine::MultisamplingBuffer::CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, 
	VkExtent2D swapchainExtent, VkFormat swapchainImageFormat) {
	Img_Func_CreateImage(
		physicalDevice,
		device,
		Image,
		ImageMemory,
		swapchainExtent.width,
		swapchainExtent.height,
		VK_IMAGE_TILING_OPTIMAL,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		swapchainImageFormat,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		Globals::gMSAAsamples,
		(uint32_t)1,
		VK_NULL_HANDLE
	);

	CreateImageView(device, swapchainImageFormat);
}

VkImage Engine::MultisamplingBuffer::GetImage() {
	return Image;
}

VkImageView Engine::MultisamplingBuffer::GetImageView() {
	return ImageView;
}

void Engine::MultisamplingBuffer::Destroy(VkDevice device) {
	vkFreeMemory(device, ImageMemory, nullptr);
	vkDestroyImage(device, Image, nullptr);
	vkDestroyImageView(device, ImageView, nullptr);
}

void Engine::CubemapTexture::CreateImageView(VkDevice device) {
	VkImageViewCreateInfo ImageViewCreateInfo{};
	ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	ImageViewCreateInfo.image = Image;
	ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ImageViewCreateInfo.subresourceRange.levelCount = 1;
	ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ImageViewCreateInfo.subresourceRange.layerCount = 6;
	ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &ImageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Image View");
	}
}

void Engine::CubemapTexture::CreateImageSampler(VkDevice device) {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &ImageSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void Engine::CubemapTexture::CreateCubemapTexture(VkDevice device, VkPhysicalDevice physicalDevice, 
	VkCommandPool commandPool, VkQueue commandBufferQueue, std::vector<std::string> paths) {

	VkBuffer cubemapBuffer;
	VkDeviceMemory cubemapDeviceMemory;
	int texWidth, texHeight, texChannels;

	Pixels = stbi_load(paths[0].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!Pixels) {
		stbi_load("assets/decoyTexture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		spdlog::warn("Failed to load cubemap face");
	}

	VkDeviceSize cubemapImageSize = texWidth * texHeight * 4 * 6;
	VkDeviceSize faceSize = texWidth * texHeight * 4;

	VulkanBuffers::Buf_Func_CreateBuffer(
		physicalDevice,
		device,
		cubemapImageSize,
		cubemapBuffer,
		cubemapDeviceMemory,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* data;
	vkMapMemory(device, cubemapDeviceMemory, 0, faceSize, 0, &data);
	memcpy(data, Pixels, faceSize);
	vkUnmapMemory(device, cubemapDeviceMemory);
	stbi_image_free(Pixels);

	for (size_t i = 1; i < 6; i++) {
		Pixels = stbi_load(paths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!Pixels) {
			stbi_load("assets/decoyTexture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			spdlog::warn("Failed to load cubemap face");
		}

		vkMapMemory(device, cubemapDeviceMemory, faceSize * i, faceSize, 0, &data);
		memcpy(data, Pixels, faceSize);
		vkUnmapMemory(device, cubemapDeviceMemory);
		stbi_image_free(Pixels);
	}

	Img_Func_CreateImage(
		physicalDevice,
		device,
		Image,
		ImageTextureMemory,
		texWidth,
		texHeight,
		VK_IMAGE_TILING_OPTIMAL,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		(uint32_t)6,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
	);

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.layerCount = 6;
	subresourceRange.levelCount = 1;

	Img_Func_TransitionImageLayout(
		device,
		commandBufferQueue,
		commandPool,
		Image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange
	);

	Img_func_CopyBufferToCubemap(
		device,
		commandBufferQueue,
		commandPool,
		Image,
		cubemapBuffer,
		texWidth,
		texHeight
	);

	Img_Func_TransitionImageLayout(
		device,
		commandBufferQueue,
		commandPool,
		Image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		subresourceRange
	);

	CreateImageView(device);
	CreateImageSampler(device);

	vkFreeMemory(device, cubemapDeviceMemory, nullptr);
	vkDestroyBuffer(device, cubemapBuffer, nullptr);
}

VkImage Engine::CubemapTexture::GetImage() {
	return Image;
}

VkImageView Engine::CubemapTexture::GetImageView() {
	return ImageView;
}

VkSampler Engine::CubemapTexture::GetImageSampler() {
	return ImageSampler;
}

void Engine::CubemapTexture::DestroyTexture(VkDevice device) {
	vkDestroySampler(device, ImageSampler, nullptr);
	vkDestroyImage(device, Image, nullptr);
	vkDestroyImageView(device, ImageView, nullptr);
	vkFreeMemory(device, ImageTextureMemory, nullptr);
}
