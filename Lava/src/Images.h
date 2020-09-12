#pragma once
#include <stb_image.h>
#include <fstream>
#include "../vendor/volk.h"
#include "DataTypes.h"
#include "Buffers.h"
#include "CommandBuffer.h"
namespace Lava{

	void createImage(
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkImage& image,
		VkDeviceMemory& imageTextureMemory,
		uint32_t width,
		uint32_t height,
		VkImageTiling tiling,
		VkMemoryPropertyFlags properties,
		VkFormat format,
		VkImageUsageFlags usage,
		VkSampleCountFlagBits samples,
		uint32_t arrayLayers,
		VkImageCreateFlags flags
	) {
		VkExtent3D extent = { width,height,1 };

		VkImageCreateInfo imageCreateInfo{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			nullptr,
			flags,
			VK_IMAGE_TYPE_2D,
			format,
			extent,
			(uint32_t)1,
			(uint32_t)arrayLayers,
			samples,
			tiling,
			usage,
			VK_SHARING_MODE_EXCLUSIVE,
			(uint32_t)0,
			(const uint32_t*)VK_NULL_HANDLE,
			VK_IMAGE_LAYOUT_UNDEFINED
		};


		if (vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Image");
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.allocationSize = memRequirements.size;
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
		VkResult result = vkAllocateMemory(device, &allocateInfo, nullptr, &imageTextureMemory);
		if (result != VK_SUCCESS) {
			std::cout << "Error code " << result << std::endl;
			throw std::runtime_error("Failed to allocate image memory");
		}
		vkBindImageMemory(device, image, imageTextureMemory, 0);
	}
	
	void transitionImageLayout(
		VkDevice device,
		VkQueue pipelineBarrierQueue,
		VkCommandPool commandPool,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageSubresourceRange subresourceRange
	) {
		CommandBuffer commandBuffer;
		commandBuffer.beginCommandBuffer(device, commandPool);
		VkImageMemoryBarrier memoryBarrier{};
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



		vkCmdPipelineBarrier(commandBuffer.get(),
			sourceStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &memoryBarrier
		);

		commandBuffer.endSubmitAndFreeCommandBuffer(device, commandPool, pipelineBarrierQueue);
	}

	void copyBufferToImage(
		VkDevice device,
		VkQueue commandBufferQueue,
		VkCommandPool commandPool,
		VkImage image,
		VkBuffer buffer,
		uint32_t width,
		uint32_t height
	) {
		CommandBuffer commandBuffer;
		commandBuffer.beginCommandBuffer(device, commandPool);
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

		vkCmdCopyBufferToImage(commandBuffer.get(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

		commandBuffer.endSubmitAndFreeCommandBuffer(device, commandPool, commandBufferQueue);
	}
	
	void copyBufferToCubemap(
		VkDevice device,
		VkQueue commandBufferQueue,
		VkCommandPool commandPool,
		VkImage image,
		VkBuffer buffer,
		uint32_t width,
		uint32_t height
	) {
		CommandBuffer commandBuffer;
		commandBuffer.beginCommandBuffer(device, commandPool);

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
			commandBuffer.get(), 
			buffer, 
			image, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			(uint32_t)bufferImageCopyVector.size(),
			bufferImageCopyVector.data()
		);
	
		commandBuffer.endSubmitAndFreeCommandBuffer(device, commandPool, commandBufferQueue);
	}

	class Texture {
		stbi_uc* pixels;
		VkImage image;
		VkDeviceMemory  imageTextureMemory;
		VkImageView imageView;
		VkSamplerCreateInfo samplerCreateInfo{};
		VkSampler imageSampler;

		void createImageView(VkDevice device) {
			VkImageViewCreateInfo ImageViewCreateInfo{};
			ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
			ImageViewCreateInfo.image = image;
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
			
			if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create Image View");
			}
		}

		void createImageSampler(VkDevice device) {
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

			if (vkCreateSampler(device, &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture sampler!");
			}
		}
	public:

		VkSampler getImageSampler() {
			return imageSampler;
		}

		VkImageView getImageView() {
			return imageView;
		}

		void destroyImage(VkDevice device) {
			vkDestroySampler(device, imageSampler, nullptr);
			vkDestroyImage(device, image, nullptr);
			vkDestroyImageView(device, imageView, nullptr);
			vkFreeMemory(device, imageTextureMemory, nullptr);
		}

		void createTexture(
			VkPhysicalDevice physicalDevice, 
			VkDevice device, 
			VkQueue commandBufferQueue, 
			VkCommandPool commandPool, 
			std::string path
		) {
			int texWidth, texHeight, texChannels;
			pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);	
			if (!pixels) {
				pixels = stbi_load("assets/noTexture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			}

			VkDeviceSize imageSize = texWidth * texHeight * 4;
			VkBuffer stagingBuffer;
			VkDeviceMemory bufferMemory;

			CreateBuffer(
				physicalDevice, 
				device, 
				imageSize, 
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
				stagingBuffer, 
				bufferMemory
			);

			void* data;
			vkMapMemory(device, bufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, (size_t)imageSize);
			vkUnmapMemory(device, bufferMemory);

			stbi_image_free(pixels);

			createImage(
				physicalDevice, 
				device,
				image,
				imageTextureMemory,
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

			transitionImageLayout(
				device, 
				commandBufferQueue, 
				commandPool, 
				image, 
				VK_IMAGE_LAYOUT_UNDEFINED, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange
			);

			copyBufferToImage(
				device, 
				commandBufferQueue,
				commandPool, 
				image, 
				stagingBuffer, 
				texWidth, 
				texHeight
			);

			transitionImageLayout(
				device, 
				commandBufferQueue, 
				commandPool, 
				image, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				subresourceRange
			);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, bufferMemory, nullptr);

			createImageView(device);
			createImageSampler(device);
		}
		
		void destroyTexture(VkDevice device) {
			destroyImage(device);
		}

	};

	class DepthImage {

		VkImage depthImage;
		VkDeviceMemory depthMemory;
		VkImageView depthImageView;
		VkFormat depthFormat;

		void createDepthImageView(VkDevice device) {
			VkImageViewCreateInfo ImageViewCreateInfo{};
			ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ImageViewCreateInfo.format = depthFormat;
			ImageViewCreateInfo.image = depthImage;
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
			if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &depthImageView) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create Image View");
			}
		}
	public:

		void createDepthBuffer(
			VkDevice logicalDevice,
			VkQueue commandBufferQueue,
			VkExtent2D swapchainExtent,
			VkPhysicalDevice physicalDevice,
			VkCommandPool commandPool
		) {
			depthFormat = VK_FORMAT_D32_SFLOAT;
			createImage(
				physicalDevice,
				logicalDevice,
				depthImage,
				depthMemory,
				swapchainExtent.width,
				swapchainExtent.height,
				VK_IMAGE_TILING_OPTIMAL,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				depthFormat,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				MSAAsamples,
				(uint32_t)1,
				VK_NULL_HANDLE
			);
			createDepthImageView(logicalDevice);

			VkImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.layerCount = 1;
			subresourceRange.levelCount = 1;

			transitionImageLayout(
				logicalDevice,
				commandBufferQueue,
				commandPool,
				depthImage,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				subresourceRange
			);
		}

		void destroy(VkDevice device) {
			vkDestroyImage(device, depthImage, nullptr);
			vkFreeMemory(device, depthMemory, nullptr);
			vkDestroyImageView(device, depthImageView, nullptr);
		}

		VkFormat getDepthFormat() {
			return depthFormat;
		}

		VkImage getDepthImage() {
			return depthImage;
		}

		VkImageView getImageView() {
			return depthImageView;
		}
	};

	class MultisamplingBuffer{
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
	public:
		void createImageView(VkDevice device,VkFormat swapchainImageFormat) {
			VkImageViewCreateInfo ImageViewCreateInfo{};
			ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ImageViewCreateInfo.format = swapchainImageFormat;
			ImageViewCreateInfo.image = image;
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

			if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create Image View");
			}
		}
		void createBuffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkExtent2D swapchainExtent,
			VkFormat swapchainImageFormat

		){
			createImage(
				physicalDevice,
				device,
				image,
				imageMemory,
				swapchainExtent.width,
				swapchainExtent.height,
				VK_IMAGE_TILING_OPTIMAL,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				swapchainImageFormat,
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				MSAAsamples,
				(uint32_t)1,
				VK_NULL_HANDLE
			);

			createImageView(device,swapchainImageFormat);
		}
		
		VkImage getImage(){
			return image;
		}
		VkImageView getImageView(){
			return imageView;
		}
		
		void destroy(VkDevice device){
			vkFreeMemory(device, imageMemory, nullptr);
			vkDestroyImage(device,image,nullptr);
			vkDestroyImageView(device,imageView,nullptr);
		}
	};

	class CubemapTexture {
		stbi_uc* pixels;
		VkImage image;
		VkDeviceMemory  imageTextureMemory;
		VkImageView imageView;
		VkSamplerCreateInfo samplerCreateInfo{};
		VkSampler imageSampler;

		void createImageView(VkDevice device) {
			VkImageViewCreateInfo ImageViewCreateInfo{};
			ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
			ImageViewCreateInfo.image = image;
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
			if (vkCreateImageView(device, &ImageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create Image View");
			}
		}

		void createImageSampler(VkDevice device) {
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

			if (vkCreateSampler(device, &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture sampler!");
			}
		}
	public:
		void createCubemapTexture(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkCommandPool commandPool,
			VkQueue commandBufferQueue,
			std::vector<std::string> paths
		){
			VkBuffer cubemapBuffer;
			VkDeviceMemory cubemapDeviceMemory;
			int texWidth, texHeight, texChannels;

			pixels = stbi_load(paths[0].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			if (!pixels) {
				throw std::runtime_error("Failed to load cubemap face");
			}
			
			VkDeviceSize cubemapImageSize = texWidth * texHeight * 4 * 6;
			VkDeviceSize faceSize = texWidth * texHeight * 4;

			CreateBuffer(
				physicalDevice,
				device,
				cubemapImageSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				cubemapBuffer,
				cubemapDeviceMemory
				);

			void *data;
			vkMapMemory(device,cubemapDeviceMemory,0, faceSize,0,&data);
			memcpy(data,pixels,faceSize);
			vkUnmapMemory(device,cubemapDeviceMemory);
			stbi_image_free(pixels);
			
			for (size_t i = 1; i < 6; i++) {
				pixels = stbi_load(paths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
				if (!pixels) {
					throw std::runtime_error("Failed to load cubemap face");
				}

				vkMapMemory(device, cubemapDeviceMemory, faceSize*i, faceSize, 0, &data);
				memcpy(data, pixels, faceSize);
				vkUnmapMemory(device, cubemapDeviceMemory);
				stbi_image_free(pixels);
			}

			createImage(
				physicalDevice,
				device,
				image,
				imageTextureMemory,
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

			transitionImageLayout(
				device,
				commandBufferQueue,
				commandPool,
				image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange
			);

			copyBufferToCubemap(
				device,
				commandBufferQueue,
				commandPool,
				image,
				cubemapBuffer,
				texWidth,
				texHeight
			);

			transitionImageLayout(
				device,
				commandBufferQueue,
				commandPool,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				subresourceRange
			);
			createImageView(device);
			createImageSampler(device);
			vkFreeMemory(device,cubemapDeviceMemory,nullptr);
			vkDestroyBuffer(device,cubemapBuffer,nullptr);
		}
		
		VkImage getImage(){
			return image;
		}

		VkImageView getImageView(){
			return imageView;
		}

		VkSampler getImageSampler(){
			return imageSampler;
		}

		void destroyTexture(VkDevice device) {
			vkDestroySampler(device, imageSampler, nullptr);
			vkDestroyImage(device, image, nullptr);
			vkDestroyImageView(device, imageView, nullptr);
			vkFreeMemory(device, imageTextureMemory, nullptr);
		}
	};
}
