#ifndef images_h
#define images_h

#define		STB_IMAGE_IMPLEMENTATION
#define		STB_IMAGE_STATIC
#include	"../../vendor/stb_image.h"
#include	<spdlog/spdlog.h>
#include	<fstream>
#include	"../../vendor/volk.h"
#include	"DataTypes.h"
#include	"Buffers.h"
#include	"CommandBuffer.h"

namespace Engine{
	void Img_Func_CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, VkImage& image,
			VkDeviceMemory& imageTextureMemory, uint32_t width, uint32_t height,
			VkImageTiling tiling, VkMemoryPropertyFlags properties, VkFormat format,
			VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t arrayLayers,
			VkImageCreateFlags flags);
	
	void Img_Func_TransitionImageLayout(VkDevice device, VkQueue pipelineBarrierQueue, VkCommandPool commandPool,
			VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkImageSubresourceRange subresourceRange);

	void Img_Func_CopyBufferToImage(VkDevice device, VkQueue copyBufferQueue, VkCommandPool commandPool,
			VkImage image, VkBuffer buffer, uint32_t width,
			uint32_t height);
	
	void Img_func_CopyBufferToCubemap(VkDevice device, VkQueue copyBufferQueue, VkCommandPool commandPool,
			VkImage image, VkBuffer buffer, uint32_t width,
			uint32_t height);

	class Texture {
		stbi_uc*			Pixels;
		VkImage				Image;
		VkDeviceMemory		ImageTextureMemory;
		VkImageView			ImageView;
		VkSampler			ImageSampler;

		void CreateImageView(VkDevice device);

		void CreateImageSampler(VkDevice device);
		public:
		VkSampler GetImageSampler();

		VkImageView GetImageView();

		void DestroyImage(VkDevice device);

		void CreateTexture(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue commandBufferQueue,
				VkCommandPool commandPool, std::string path);
		
		void DestroyTexture(VkDevice device);
	};

	class DepthImage {
		VkImage			vDepthImage;
		VkDeviceMemory  DepthImageMemory;
		VkImageView		DepthImageView;
		VkFormat		DepthFormat;

		void CreateDepthImageView(VkDevice device);
		public:
		void CreateDepthBuffer(VkDevice logicalDevice, VkQueue commandBufferQueue, VkExtent2D swapchainExtent,
				VkPhysicalDevice physicalDevice, VkCommandPool commandPool);

		void Destroy(VkDevice device);

		VkFormat GetDepthFormat();

		VkImage GetDepthImage();

		VkImageView GetImageView();
		
	};	

	class MultisamplingBuffer{
		VkImage Image;
		VkDeviceMemory ImageMemory;
		VkImageView ImageView;
		public:
		void CreateImageView(VkDevice device, VkFormat swapchainImageFormat);

		void CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
				VkExtent2D swapchainExtent, VkFormat swapchainImageFormat);
		
		VkImage GetImage();
		
		VkImageView GetImageView();	

		void Destroy(VkDevice device);

	};

	class CubemapTexture {
		stbi_uc*		Pixels;
		VkImage			Image;
		VkDeviceMemory  ImageTextureMemory;
		VkImageView		ImageView;
		VkSampler		ImageSampler;
		
		void CreateImageView(VkDevice device);

		void CreateImageSampler(VkDevice device);
		public:
		void CreateCubemapTexture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
				VkQueue commandBufferQueue, std::vector<std::string> paths);
		
		VkImage GetImage();

		VkImageView GetImageView();

		VkSampler GetImageSampler();

		void DestroyTexture(VkDevice device);

	};

	namespace Globals{
		extern DepthImage gDepthImage;
		extern MultisamplingBuffer gMultisamplingBuffer;
	}
}
#endif