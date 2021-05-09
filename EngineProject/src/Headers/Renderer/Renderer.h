#pragma once
#define	VK_NO_PROTOTYPES
#define GLM_FORCE_CXX11 


#include "../../vendor/imgui.h"
#include "../../vendor/imgui_impl_glfw.h"
#include "../../vendor/imgui_impl_vulkan.h"
#include "../../vendor/imgui_impl_win32.h"
#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/quaternion.hpp"
#include "../../vendor/glm/gtx/quaternion.hpp"
#include "../../vendor/volk.h"
//#include <windows.h>
#include <array>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include "../Entities.h"
#include "../Scene.h"
#include "VkInstance.h"
#include "PhysicalDevice.h"
#include "DataTypes.h"
#include "Device.h"
#include "Buffers.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorPools.h"
#include "DescriptorSetLayouts.h"
#include "Pipelines.h"
#include "Surface.h"
#include "Swapchain.h"
#include "SyncObjects.h"
#include "Images.h"

namespace Engine {

    class Renderer
    {
    private:
        //  Настройка и создание фреймбуферов
        std::vector<VkFramebuffer> createFramebuffers(VkDevice device, VkRenderPass renderPass, std::vector<VkImageView> swapchainImageViews,
            VkImageView depthImageView, VkImageView multisamplingImageView, VkExtent2D swapchainExtent);

        std::vector<VkFramebuffer> createOffscreenFramebuffers(VkDevice device, VkRenderPass renderPass, int swapchainImageViewCount,
            VkImageView depthImageView, int ShadowMapDimensions);

    public:
        Instance instance;
        PhysicalDevice physicalDevice;
        Device device;
        Surface surface;
        Swapchain swapchain;
        SyncObjects syncObjects;
        CommandPool commandPool;
        std::vector<VkFence> imagesInFlight;
    public:
        RenderPass renderPass;
        GraphicsPipelineForMesh graphicsPipelineForMesh;
        GraphicsPipelineForCubemapObjects graphicsPipelineForCubemapObjects;
        GraphicsPipelineForRigidBodyMesh graphicsPipelineForRigidBodyMesh;
        DepthImage depthImage;
        MultisamplingBuffer multisamplingBuffer;
    public:
        //Shadows
        DescriptorSetLayoutForShadowMap setLayoutForShadowMap;
        DescriptorPoolForShadowMap descriptorPoolForShadowMap;
        ShadowMapOffscreenRenderPass offscreenRenderpass;
        GraphicsPipelineForShadowMap graphicsPipelineForShadowMap;
        DepthImageShadowMap depthImageShadowMap;
        std::vector<VkFramebuffer> offscreenFramebuffers;
    public:
        DescriptorPoolForMesh descriptorPoolForMesh;
        DescriptorPoolForRigidBodyMesh descriptorPoolForRigidBodyMesh;
        DescriptorPoolForCubemapObjects descriptorPoolForCubemapObjects;
        DescriptorPoolForImgui descriptorPoolForImgui;
    public:
        DescriptorSetLayoutForMesh setLayoutForMesh;
        DescriptorSetLayoutForRigidBodyMesh setLayoutForRigidBodyMesh;
        DescriptorSetLayoutForCubemapObjects setLayoutForCubemapObjects;
    public:
        VkRect2D   rendererScissors;
        VkViewport rendererViewport;

#define MAX_FRAMES 2
        //  Буфер с коммандами для отрисовки сцены
        CommandBuffer drawCommandBuffer;

        uint32_t imageIndex;
        uint32_t currentFrame = 0;

        int newNumberOfEntities = 0;
        int oldNumberOfEntities = 0;

        //  Инициализация объектов Vulkan
        void initVulkan(HWND hwnd, HINSTANCE hInstance);

        void recreateSwapchain();   

        void DrawScene(ImDrawData* drawData, Scene* scene, Camera camera);

      
        void FlushDrawingBuffer();

        void clear();

    };

    extern Renderer renderer;
}


