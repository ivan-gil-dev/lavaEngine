#pragma once
#define	VK_NO_PROTOTYPES
#define	GLM_FORCE_RADIANS

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
            VkImageView depthImageView, VkImageView multisamplingImageView, VkExtent2D swapchainExtent) {

            std::vector<VkFramebuffer> framebuffers;
            framebuffers.resize(swapchainImageViews.size());
            for (size_t i = 0; i < swapchainImageViews.size(); i++) {

                //	Набор изображений для вывода во фреймбуфер
                VkImageView attachments[] = {
                    //  msaa
                        multisamplingImageView,
                    //  Z-буфер
                        depthImageView,
                    //  Изображение из swapchain
                        swapchainImageViews[i],
                };

                VkFramebufferCreateInfo framebufferCreateInfo{};

                //  Размер фреймбуфера указывается в соответствии с размером изображения,
                //  получаемым из свапчейна
                framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferCreateInfo.renderPass = renderPass;
                framebufferCreateInfo.height = swapchainExtent.height;
                framebufferCreateInfo.width = swapchainExtent.width;
                framebufferCreateInfo.pAttachments = attachments;
                framebufferCreateInfo.attachmentCount = (uint32_t)3;
                framebufferCreateInfo.layers = 1;
                

                if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create framebuffer");
                }
            }
            return framebuffers;
        }

    public:
        Instance instance;
        PhysicalDevice physicalDevice;
        Device device;
        Surface surface;
        Swapchain swapchain;
        SyncObjects syncObjects;
        CommandPool commandPool;
    public:
        RenderPass renderPass;
        GraphicsPipelineForMesh graphicsPipelineForMesh;
        GraphicsPipelineForCubemapObjects graphicsPipelineForCubemapObjects;
        GraphicsPipelineForRigidBodyMesh gGraphicsPipelineForRigidBodyMesh;
        DepthImage depthImage;
        MultisamplingBuffer multisamplingBuffer;
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

        uint32_t currentFrame = 0;

        int newNumberOfEntities = 0;
        int oldNumberOfEntities = 0;

        //  Инициализация объектов Vulkan
        void initVulkan(HWND hwnd, HINSTANCE hInstance) {

            instance.createInstance();

            physicalDevice.PickPhysicalDevice(instance.get());

            device.CreateDevice(physicalDevice.Get(), physicalDevice.GetQueueIndices());

            surface.CreateSurface(hwnd, hInstance, instance.get());

            rendererViewport.x = 0;
            rendererViewport.y = 0;
            rendererViewport.height = static_cast<float>(Globals::gHeight);
            rendererViewport.width = static_cast<float>(Globals::gWidth);
            rendererViewport.minDepth = 0.0f;
            rendererViewport.maxDepth = 1.0f;

           swapchain.CreateSwapchain(physicalDevice.Get(),
                device.Get(),
                surface.Get(),
                physicalDevice.GetQueueIndices());

           swapchain.CreateImageViews(device.Get());

            commandPool.CreateCommandPool(physicalDevice.GetQueueIndices(),
                device.Get());

            rendererScissors.extent = swapchain.GetInfo().imageExtent;


            depthImage.CreateDepthBuffer(
                device.Get(),
                device.GetGraphicsQueue(),
                swapchain.GetInfo().imageExtent, physicalDevice.Get(),
                commandPool.Get()
            );

            multisamplingBuffer.CreateBuffer(
                device.Get(),
                physicalDevice.Get(),
                swapchain.GetInfo().imageExtent,
                swapchain.GetInfo().imageFormat
            );


            setLayoutForMesh.CreateDescriptorSetLayout(device.Get());

            setLayoutForRigidBodyMesh.CreateDescriptorSetLayout(device.Get());

            setLayoutForCubemapObjects.CreateDescriptorSetLayout(device.Get());

            renderPass.CreateRenderPass(device.Get(),
                swapchain.GetInfo(),
                depthImage.GetDepthFormat());

            graphicsPipelineForMesh.CreateGraphicsPipeline(
                device.Get(),
                swapchain.GetInfo(),
                setLayoutForMesh.Get(),
                renderPass.GetRenderPass()
            );

            graphicsPipelineForCubemapObjects.CreateGraphicsPipeline(
                device.Get(),
                swapchain.GetInfo(),
                setLayoutForCubemapObjects.Get(),
                renderPass.GetRenderPass()
            );

            gGraphicsPipelineForRigidBodyMesh.CreateGraphicsPipeline(
                device.Get(),
                swapchain.GetInfo(),
                setLayoutForRigidBodyMesh.Get(),
                renderPass.GetRenderPass()
            );

            swapchain.SetFramebuffers(
                createFramebuffers(
                    device.Get(),
                    renderPass.GetRenderPass(),
                    *swapchain.PGetImageViews(),
                    depthImage.GetImageView(),
                    multisamplingBuffer.GetImageView(),
                    swapchain.GetInfo().imageExtent
                )
            );

            //drawCommandBuffers.resize(swapchain.GetSwapchainFramebuffers().size());

            descriptorPoolForMesh.CreateDescriptorPool(
                device.Get(),
                *swapchain.PGetImageViews()
            );

            descriptorPoolForRigidBodyMesh.CreateDescriptorPool(
                device.Get(),
                *swapchain.PGetImageViews()
            );

            descriptorPoolForImgui.CreateDescriptorPool(
                device.Get(),
                *swapchain.PGetImageViews()
            );

            descriptorPoolForCubemapObjects.CreateDescriptorPool(
                device.Get(),
                *swapchain.PGetImageViews()
            );

            syncObjects.CreateSyncObjects(device.Get());

        }

        void recreateSwapchain() {
            vkDeviceWaitIdle(device.Get());
            depthImage.Destroy(device.Get());
            multisamplingBuffer.Destroy(device.Get());

            swapchain.DestroySwapchainObjects(device.Get());

            swapchain.CreateSwapchain(physicalDevice.Get(),
                device.Get(),
                surface.Get(),
                physicalDevice.GetQueueIndices());

            swapchain.CreateImageViews(device.Get());

            depthImage.CreateDepthBuffer(
                device.Get(),
                device.GetGraphicsQueue(),
                swapchain.GetInfo().imageExtent, physicalDevice.Get(),
                commandPool.Get()
            );

            multisamplingBuffer.CreateBuffer(
                device.Get(),
                physicalDevice.Get(),
                swapchain.GetInfo().imageExtent,
                swapchain.GetInfo().imageFormat
            );

            swapchain.SetFramebuffers(
                createFramebuffers(
                    device.Get(),
                    renderPass.GetRenderPass(),
                    *swapchain.PGetImageViews(), depthImage.GetImageView(),
                    multisamplingBuffer.GetImageView(),
                    swapchain.GetInfo().imageExtent
                )
            );
            
            rendererScissors.extent = swapchain.GetInfo().imageExtent;

        }

        void DrawScene(ImDrawData* drawData, Scene* scene, Camera camera){
            uint32_t imageIndex;
            newNumberOfEntities = scene->pGetVectorOfEntities()->size();

            vkWaitForFences(device.Get(), 1, &syncObjects.GetFences()[currentFrame], VK_TRUE, UINT64_MAX);
            vkResetFences(device.Get(), 1, &syncObjects.GetFences()[currentFrame]);
          
            VkResult result;
            result = vkAcquireNextImageKHR(
                device.Get(),
                swapchain.Get(),
                UINT64_MAX,
                syncObjects.GetImageAvailableSemaphores()[currentFrame],
                VK_NULL_HANDLE,
                &imageIndex
            );

            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                recreateSwapchain();
                return;
            }

            std::vector<Entity*>* entities = scene->pGetVectorOfEntities();
            for (size_t i = 0; i < entities->size(); i++) {
                if (entities->at(i)->GetEntityType() == ENTITY_TYPE_GAME_OBJECT) {
                    ((GameObject*)entities->at(i))->UpdateUniforms(imageIndex, device.Get(), camera, scene->GetVectorOfSpotlightAttributes(),scene->GetVectorOfDirectionalLightAttributes());
                }
                if (entities->at(i)->GetEntityType() == ENTITY_TYPE_CUBEMAP_OBJECT) {
                    ((CubemapObject*)entities->at(i))->UpdateUniforms(imageIndex, device.Get(), camera);
                }
                if (entities->at(i)->GetEntityType() == ENTITY_TYPE_POINTLIGHT_OBJECT) {
                    ((PointLightObject*)entities->at(i))->UpdateUniforms(imageIndex, device.Get(), camera);
                }
            }


            VkPipelineStageFlags stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            VkSemaphore waitSemaphores[] = { syncObjects.GetImageAvailableSemaphores()[currentFrame] };
            VkSemaphore signalSemaphores[] = { syncObjects.GetImageRenderedSemaphores()[currentFrame] };

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = renderPass.GetRenderPass();
            renderPassBeginInfo.renderArea.extent = swapchain.GetInfo().imageExtent;
            renderPassBeginInfo.renderArea.offset = { 0,0 };
            renderPassBeginInfo.framebuffer = swapchain.GetSwapchainFramebuffers()[imageIndex];

            VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
            VkClearValue depthClearColor = { 1.0,0.0f };
            VkClearValue resolveColor = { 0.0f,0.0f,0.0f,1.0f };
            VkClearValue clearColors[] = {
                clearColor, depthClearColor, resolveColor
            };

            renderPassBeginInfo.pClearValues = clearColors;
            renderPassBeginInfo.clearValueCount = 3;

            drawCommandBuffer.AllocateCommandBuffer(device.Get(), commandPool.Get());
            drawCommandBuffer.BeginCommandBuffer();

            vkCmdBeginRenderPass(drawCommandBuffer.Get(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            for (size_t i = 0; i < entities->size(); i++) {
                entities->at(i)->Draw(
                    drawCommandBuffer.Get(),
                    imageIndex
                );
            }

            if (ENABLE_IMGUI) {
                ImGui_ImplVulkan_RenderDrawData(drawData, drawCommandBuffer.Get());
            }

            vkCmdEndRenderPass(drawCommandBuffer.Get());

            drawCommandBuffer.EndCommandBuffer();
                         
            VkCommandBuffer commandBuffers[] = { drawCommandBuffer.Get() };

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = (uint32_t)1;
            submitInfo.pCommandBuffers = commandBuffers;
            submitInfo.pSignalSemaphores = signalSemaphores;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pWaitDstStageMask = stages;

            if (vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, syncObjects.GetFences()[currentFrame]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to submit info");
            }

            vkWaitForFences(device.Get(), 1, &syncObjects.GetFences()[currentFrame], VK_FALSE, UINT64_MAX);

            drawCommandBuffer.FreeCommandBuffer(device.Get(), commandPool.Get());

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pWaitSemaphores = signalSemaphores;
            presentInfo.waitSemaphoreCount = 1;

            VkSwapchainKHR swapchains[] = { swapchain.Get() };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapchains;
            presentInfo.pResults = 0;
            presentInfo.pImageIndices = &imageIndex;

            result = vkQueuePresentKHR(device.GetGraphicsQueue(), &presentInfo);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                recreateSwapchain();
                return;
            }

            currentFrame = (currentFrame + 1) % MAX_FRAMES;
        }

        void clear() {
            drawCommandBuffer.FreeCommandBuffer(device.Get(), commandPool.Get());
            setLayoutForMesh.Destroy(device.Get());
            setLayoutForCubemapObjects.Destroy(device.Get());
            setLayoutForRigidBodyMesh.Destroy(device.Get());
            depthImage.Destroy(device.Get());
            descriptorPoolForMesh.Destroy(device.Get());
            descriptorPoolForRigidBodyMesh.Destroy(device.Get());
            descriptorPoolForImgui.Destroy(device.Get());
            descriptorPoolForCubemapObjects.Destroy(device.Get());
            syncObjects.DestroySyncObjects(device.Get());
            renderPass.Destroy(device.Get());
            multisamplingBuffer.Destroy(device.Get());
            graphicsPipelineForMesh.DestroyPipelineObjects(device.Get());
            graphicsPipelineForCubemapObjects.DestroyPipelineObjects(device.Get());
            gGraphicsPipelineForRigidBodyMesh.DestroyPipelineObjects(device.Get());
            commandPool.Destroy(device.Get());
            swapchain.DestroySwapchainObjects(device.Get());
            surface.Destroy(instance.get());
            device.Destroy();
            instance.destroy();
        }

    };

    extern Renderer renderer;
}


