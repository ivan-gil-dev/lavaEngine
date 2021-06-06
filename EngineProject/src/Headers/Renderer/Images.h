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

namespace Engine {
#define MAX_MATERIALS 64

    //Создание изображения
    void Img_Func_CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, VkImage& image,
        VkDeviceMemory& imageTextureMemory, uint32_t width, uint32_t height,
        VkImageTiling tiling, VkMemoryPropertyFlags properties, VkFormat format,
        VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t arrayLayers,
        VkImageCreateFlags flags);

    //Pipeline Barrier (указать на какой этап конвеера будет перемещено изображение)
    void Img_Func_TransitionImageLayout(VkDevice device, VkQueue pipelineBarrierQueue, VkCommandPool commandPool,
        VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
        VkImageSubresourceRange subresourceRange);

    //Копирование буфера в изображение
    void Img_Func_CopyBufferToImage(VkDevice device, VkQueue copyBufferQueue, VkCommandPool commandPool,
        VkImage image, VkBuffer buffer, uint32_t width,
        uint32_t height);

    //Копирование буфера в изображение skybox (буфер из 6 изображений)
    void Img_func_CopyBufferToCubemap(VkDevice device, VkQueue copyBufferQueue, VkCommandPool commandPool,
        VkImage image, VkBuffer buffer, uint32_t width,
        uint32_t height);

    //Класс текстуры//
    class Texture {
        //Байты данных загруженного изображения//
        stbi_uc* Pixels;

        VkImage				Image;
        VkDeviceMemory		ImageTextureMemory;

        //Вид на изображение//
        VkImageView			ImageView;

        //Сэмплер//
        VkSampler			ImageSampler;

        //Путь к текстуре//
        std::string			texturePath;

        void CreateImageView(VkDevice device);

        void CreateImageSampler(VkDevice device);
    public:
        std::string GetTexturePath();

        VkSampler GetImageSampler();

        VkImageView GetImageView();

        void DestroyImage(VkDevice device);

        void CreateTexture(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue commandBufferQueue,
            VkCommandPool commandPool, std::string path);

        void DestroyTexture(VkDevice device);
    };

    //Z-buffer//
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

    //Карта теней//
    class DepthImageShadowMap {
        VkImage			vDepthImage;
        VkDeviceMemory  DepthImageMemory;
        VkImageView		DepthImageView;
        VkFormat		DepthFormat;
        VkSampler       DepthSampler;
        int				ShadowMapDimensions;

        std::vector<std::vector<VulkanBuffers::UniformBuffer>> b0_MVP;
        std::vector<std::vector<VkDescriptorSet>> DescriptorSets;

        void CreateImageSampler(VkDevice device);
        void CreateDepthImageView(VkDevice device);
    public:
        void CreateDepthBuffer(VkDevice logicalDevice, VkQueue commandBufferQueue,
            VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
            int swapchainImageViewCount, VkDescriptorPool pool, VkDescriptorSetLayout* pSetLayout);

        void UpdateDescriptorSets(VkDevice device, std::vector<VulkanBuffers::UniformBuffer> UniformBuffers);

        void Destroy(VkDevice device, VkDescriptorPool pool);

        void UpdateUniformBuffers(uint32_t imageIndex, VkDevice device, glm::vec3 lightPos, std::vector<DataTypes::MVP_t> MVPs);

        std::vector<VkDescriptorSet> GetDescriptorSetsByIndex(int index);

        VkFormat GetDepthFormat();

        VkImage GetDepthImage();

        VkSampler GetImageSampler();

        VkImageView GetImageView();

        int GetShadowMapDimensions();
    };

    //MSAA буфер//
    class MultisamplingBuffer {
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

    //Текстура для скайбокса//
    class CubemapTexture {
        stbi_uc* Pixels;
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

    /*namespace Globals{
        extern DepthImage gDepthImage;
        extern MultisamplingBuffer gMultisamplingBuffer;
    }*/
}
#endif