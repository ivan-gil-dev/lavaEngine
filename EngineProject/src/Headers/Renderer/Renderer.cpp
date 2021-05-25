#include "Renderer.h"

namespace Engine {
    Renderer renderer;

    std::vector<VkFramebuffer> Renderer::createFramebuffers(VkDevice device, VkRenderPass renderPass, std::vector<VkImageView> swapchainImageViews, VkImageView depthImageView, VkImageView multisamplingImageView, VkExtent2D swapchainExtent)
    {
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

    std::vector<VkFramebuffer> Renderer::createOffscreenFramebuffers(VkDevice device, VkRenderPass renderPass, int swapchainImageViewCount, VkImageView depthImageView, int ShadowMapDimensions)
    {
        std::vector<VkFramebuffer> framebuffers;
        framebuffers.resize(swapchainImageViewCount);
        for (size_t i = 0; i < swapchainImageViewCount; i++) {

            //	Набор изображений для вывода во фреймбуфер
            VkImageView attachments[] = {
                //  Z-буфер
                depthImageView
            };

            VkFramebufferCreateInfo framebufferCreateInfo{};

            //  Размер фреймбуфера указывается в соответствии с размером изображения,
            //  получаемым из свапчейна
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = renderPass;
            framebufferCreateInfo.height = (uint32_t)ShadowMapDimensions;
            framebufferCreateInfo.width = (uint32_t)ShadowMapDimensions;
            framebufferCreateInfo.pAttachments = attachments;
            framebufferCreateInfo.attachmentCount = (uint32_t)1;
            framebufferCreateInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer");
            }
        }
        return framebuffers;
    }

    void Renderer::initVulkan(HWND hwnd, HINSTANCE hInstance)
    {
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

        MAX_FRAMES = (int)swapchain.PGetImageViews()->size();

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

        graphicsPipelineForRigidBodyMesh.CreateGraphicsPipeline(
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

        descriptorPoolForMesh.CreateDescriptorPool(
            device.Get(),
            (int)swapchain.PGetImageViews()->size()
        );

        descriptorPoolForRigidBodyMesh.CreateDescriptorPool(
            device.Get(),
            (int)swapchain.PGetImageViews()->size()
        );

        descriptorPoolForImgui.CreateDescriptorPool(
            device.Get(),
            (int)swapchain.PGetImageViews()->size()
        );

        descriptorPoolForCubemapObjects.CreateDescriptorPool(
            device.Get(),
            (int)swapchain.PGetImageViews()->size()
        );

        syncObjects.CreateSyncObjects(device.Get(), MAX_FRAMES);


        setLayoutForShadowMap.CreateDescriptorSetLayout(device.Get());

        descriptorPoolForShadowMap.CreateDescriptorPool(device.Get(), (int)swapchain.PGetImageViews()->size());

        offscreenRenderpass.CreateRenderPass(device.Get(),
            swapchain.GetInfo(),
            depthImage.GetDepthFormat());

        graphicsPipelineForShadowMap.CreateGraphicsPipeline(
            device.Get(),
            swapchain.GetInfo(),
            setLayoutForShadowMap.Get(),
            offscreenRenderpass.GetRenderPass()
        );

        depthImageShadowMap.CreateDepthBuffer(
            device.Get(),
            device.GetGraphicsQueue(),
            physicalDevice.Get(),
            commandPool.Get(),
            (int)swapchain.PGetImageViews()->size(),
            descriptorPoolForShadowMap.Get(),
            setLayoutForShadowMap.pGet()
        );

        offscreenFramebuffers = createOffscreenFramebuffers(
            device.Get(),
            offscreenRenderpass.GetRenderPass(),
            (int)swapchain.PGetImageViews()->size(),
            depthImageShadowMap.GetImageView(),
            depthImageShadowMap.GetShadowMapDimensions()
        );

        drawCommandBuffer.resize(swapchain.PGetImageViews()->size());
        drawCommandBufferForImgui.resize(swapchain.PGetImageViews()->size());
        drawCommandPool.resize(swapchain.PGetImageViews()->size());
        checkBuild.resize(swapchain.PGetImageViews()->size());
        for (size_t i = 0; i < drawCommandBuffer.size(); i++)
        {
            drawCommandPool[i].CreateCommandPool(physicalDevice.GetQueueIndices(), device.Get());
            drawCommandBuffer[i].AllocateCommandBuffer(device.Get(), drawCommandPool[i].Get());
            drawCommandBufferForImgui[i].AllocateCommandBuffer(device.Get(), drawCommandPool[i].Get());
            checkBuild[i] = false;
        }


        old.rendererScissors = rendererScissors;
        old.rendererViewport = rendererViewport;
        old.entityCount = 0;
        old.oldStates = Globals::states;

    }

    void Renderer::recreateSwapchain()
    {
        vkDeviceWaitIdle(device.Get());
        depthImage.Destroy(device.Get());
        multisamplingBuffer.Destroy(device.Get());

        swapchain.DestroySwapchainObjects(device.Get());

        swapchain.CreateSwapchain(physicalDevice.Get(),
            device.Get(),
            surface.Get(),
            physicalDevice.GetQueueIndices());

        Globals::gHeight = swapchain.GetInfo().imageExtent.height;
        Globals::gWidth = swapchain.GetInfo().imageExtent.width;

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

    void Renderer::DrawScene(ImDrawData* drawData, Scene* scene, Camera camera)
    {
        {
            if (old.entityCount != (int)scene->pGetVectorOfEntities()->size())
            {
                //std::cout << "Rebuild1" << std::endl;
                RebuildBuffers();
            }
            else
            if (old.rendererScissors.extent.height != rendererScissors.extent.height ||
                old.rendererScissors.extent.width != rendererScissors.extent.width ||
                old.rendererScissors.offset.x != rendererScissors.offset.x ||
                old.rendererScissors.offset.y != rendererScissors.offset.y
                )
            {
                old.rendererScissors = rendererScissors;
                //std::cout << "Rebuild2" << std::endl;
                RebuildBuffers();
            }

            else
            if (old.rendererViewport.height != rendererViewport.height ||
                old.rendererViewport.width != rendererViewport.width ||
                old.rendererViewport.x != rendererViewport.x ||
                old.rendererViewport.y != rendererViewport.y
                )
            {
                //std::cout << "Rebuild3" << std::endl;
                old.rendererViewport = rendererViewport;
                RebuildBuffers();
            }
            else
            if (!(old.oldStates == Globals::states)) {
                old.oldStates = Globals::states;
                RebuildBuffers();

            }



            old.entityCount = (int)scene->pGetVectorOfEntities()->size();
        }
        
       
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
                ((GameObject*)entities->at(i))->UpdateUniforms(imageIndex, device.Get(), camera,
                    *scene->pGetVectorOfSpotlightAttributes(), *scene->pGetVectorOfDirectionalLightAttributes());
            }
            if (entities->at(i)->GetEntityType() == ENTITY_TYPE_CUBEMAP_OBJECT) {
                ((CubemapObject*)entities->at(i))->UpdateUniforms(imageIndex, device.Get(), camera);
            }
            if (entities->at(i)->GetEntityType() == ENTITY_TYPE_POINTLIGHT_OBJECT) {
                ((PointLightObject*)entities->at(i))->UpdateUniforms(imageIndex, device.Get(), camera);
            }
        }

        

        std::vector<DataTypes::MVP_t> MVPs;
        for (size_t i = 0; i < entities->size(); i++)
        {
            if (entities->at(i)->GetEntityType() == ENTITY_TYPE_GAME_OBJECT)
            {
                if (((GameObject*)entities->at(i))->pGetComponent<Mesh*>() != nullptr)
                {
                    MVPs.push_back(*((GameObject*)entities->at(i))->pGetComponent<Mesh*>()->pGetMVP());
                }
            }
        }

        if (scene->pGetVectorOfSpotlightAttributes()->size() != 0)
        {
            depthImageShadowMap.UpdateUniformBuffers(imageIndex, device.Get(), scene->pGetVectorOfSpotlightAttributes()->at(0)->lightPosition, MVPs);
        }
        else {
            depthImageShadowMap.UpdateUniformBuffers(imageIndex, device.Get(), glm::vec3(0, 0, 0), MVPs);
        }

        MVPs.clear();

        BuildCommandBuffers(drawData, scene);

        if (ENABLE_IMGUI)
        {
            drawCommandBufferForImgui[currentFrame].BeginCommandBuffer();
            VkRenderPassBeginInfo renderPassBeginInfo;
            renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = renderPass.GetRenderPass();
            renderPassBeginInfo.renderArea.extent = swapchain.GetInfo().imageExtent;
            renderPassBeginInfo.renderArea.offset = { 0,0 };
            renderPassBeginInfo.framebuffer = swapchain.GetSwapchainFramebuffers()[imageIndex];

            VkClearValue clearColor = { 0.0f,0.0f,0.0f,1.0f };
            VkClearValue depthClearColor = { 1.0,0.0f };
            VkClearValue resolveColor = { 0.0f,0.0f,0.0f,1.0f };
            VkClearValue clearColors[] = {
                clearColor,depthClearColor,resolveColor
            };

            renderPassBeginInfo.pClearValues = clearColors;
            renderPassBeginInfo.clearValueCount = 3;

            vkCmdBeginRenderPass(drawCommandBufferForImgui[currentFrame].Get(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            ImGui_ImplVulkan_RenderDrawData(drawData, drawCommandBufferForImgui[currentFrame].Get());
            vkCmdEndRenderPass(drawCommandBufferForImgui[currentFrame].Get());


            drawCommandBufferForImgui[currentFrame].EndCommandBuffer();
        }
        
        VkCommandBuffer commandBuffers[] = { drawCommandBuffer[currentFrame].Get(), drawCommandBufferForImgui[currentFrame].Get()};
        VkPipelineStageFlags stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore waitSemaphores[] = { syncObjects.GetImageAvailableSemaphores()[currentFrame] };
        VkSemaphore signalSemaphores[] = { syncObjects.GetImageRenderedSemaphores()[currentFrame] };
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = (uint32_t)2;
        submitInfo.pCommandBuffers = commandBuffers;
        submitInfo.pSignalSemaphores = signalSemaphores;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pWaitDstStageMask = stages;

        if (vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, syncObjects.GetFences()[currentFrame]) != VK_SUCCESS) {
            spdlog::warn(("Failed to submit info while drawing!"));
            return;
        }

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
        if (result == VK_SUBOPTIMAL_KHR) {
            recreateSwapchain();
            return;
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES;
    }

    void Renderer::RebuildBuffers()
    {
        for (size_t i = 0; i < checkBuild.size(); i++)
        {
            checkBuild[i] = false;
        }
    }

    void Renderer::WaitForDrawFences()
    {
        vkWaitForFences(device.Get(), syncObjects.GetFences().size(), syncObjects.GetFences().data(), VK_TRUE, UINT64_MAX);
    }

    void Renderer::BuildCommandBuffers(ImDrawData* drawData, Scene* scene)
    {
        if (!BlockBuilding)
        {
            if (!checkBuild[currentFrame])
            {
                //std::cout << "CheckBuild" << std::endl;
                drawCommandBuffer[currentFrame].BeginCommandBuffer();
                VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
                //--------shadowmapping---------
                {

                    VkClearValue clVal = { 1.f,0 };
                    VkClearValue values[] = { clVal };


                    renderPassBeginInfo.renderPass = offscreenRenderpass.GetRenderPass();
                    renderPassBeginInfo.framebuffer = offscreenFramebuffers[imageIndex];
                    renderPassBeginInfo.renderArea.extent.width = depthImageShadowMap.GetShadowMapDimensions();
                    renderPassBeginInfo.renderArea.extent.height = depthImageShadowMap.GetShadowMapDimensions();
                    renderPassBeginInfo.clearValueCount = 1;
                    renderPassBeginInfo.pClearValues = values;

                    vkCmdBeginRenderPass(drawCommandBuffer[currentFrame].Get(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


                    VkViewport viewport{};
                    viewport.height = (float)depthImageShadowMap.GetShadowMapDimensions();
                    viewport.width = (float)depthImageShadowMap.GetShadowMapDimensions();
                    viewport.minDepth = 0;
                    viewport.maxDepth = 1;
                    vkCmdSetViewport(drawCommandBuffer[currentFrame].Get(), 0, 1, &viewport);


                    VkRect2D rect{};
                    rect.extent.height = depthImageShadowMap.GetShadowMapDimensions();
                    rect.extent.width = depthImageShadowMap.GetShadowMapDimensions();
                    rect.offset.x = 0;
                    rect.offset.y = 0;

                    vkCmdSetScissor(drawCommandBuffer[currentFrame].Get(), 0, 1, &rect);

                    vkCmdSetDepthBias(
                        drawCommandBuffer[currentFrame].Get(),
                        5.05f,
                        0.0f,
                        1.75f);

                    if (Globals::states.drawShadows)
                    {
                        for (size_t j = 0, i = 0; i < scene->pGetVectorOfEntities()->size(); i++) {


                            if (scene->pGetVectorOfEntities()->at(i)->GetEntityType() == ENTITY_TYPE_GAME_OBJECT)
                            {
                                if (((GameObject*)scene->pGetVectorOfEntities()->at(i))->pGetComponent<Mesh*>() != nullptr)
                                {
                                    std::vector<VkDescriptorSet> descriptorSets = depthImageShadowMap.GetDescriptorSetsByIndex((int)j);
                                    ((GameObject*)scene->pGetVectorOfEntities()->at(i))->DrawShadowMaps(
                                        drawCommandBuffer[currentFrame].Get(),
                                        imageIndex,
                                        descriptorSets
                                    );
                                    j++;
                                }
                            }
                        }
                    }


                    vkCmdEndRenderPass(drawCommandBuffer[currentFrame].Get());
                    checkBuild[currentFrame] = true;
                    //-----------------------------------------------------------------------------------
                }

                renderPassBeginInfo = {};
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
                vkCmdBeginRenderPass(drawCommandBuffer[currentFrame].Get(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                if (ENABLE_DYNAMIC_VIEWPORT) {
                    vkCmdSetViewport(drawCommandBuffer[currentFrame].Get(), 0, 1, &renderer.rendererViewport);
                    vkCmdSetScissor(drawCommandBuffer[currentFrame].Get(), 0, 1, &renderer.rendererScissors);
                }


                for (size_t i = 0; i < scene->pGetVectorOfEntities()->size(); i++) {
                    scene->pGetVectorOfEntities()->at(i)->Draw(
                        drawCommandBuffer[currentFrame].Get(),
                        imageIndex
                    );
                }


                vkCmdEndRenderPass(drawCommandBuffer[currentFrame].Get());

                drawCommandBuffer[currentFrame].EndCommandBuffer();
            }
        }
       
    }

    void Renderer::clear()
    {   
        for (size_t i = 0; i < drawCommandBuffer.size(); i++)
        {
            
            drawCommandBuffer[i].FreeCommandBuffer(device.Get(), drawCommandPool[i].Get());
            drawCommandBufferForImgui[i].FreeCommandBuffer(device.Get(), drawCommandPool[i].Get());
            
        }
        
        
        for (int i = 0; i < offscreenFramebuffers.size(); i++)
        {
            vkDestroyFramebuffer(device.Get(), offscreenFramebuffers[i], nullptr);
        }
        depthImageShadowMap.Destroy(device.Get(), descriptorPoolForShadowMap.Get());
        setLayoutForShadowMap.Destroy(device.Get());
        setLayoutForMesh.Destroy(device.Get());
        setLayoutForCubemapObjects.Destroy(device.Get());
        setLayoutForRigidBodyMesh.Destroy(device.Get());
        depthImage.Destroy(device.Get());
        descriptorPoolForShadowMap.Destroy(device.Get());
        descriptorPoolForMesh.Destroy(device.Get());
        descriptorPoolForRigidBodyMesh.Destroy(device.Get());
        descriptorPoolForImgui.Destroy(device.Get());
        descriptorPoolForCubemapObjects.Destroy(device.Get());
        syncObjects.DestroySyncObjects(device.Get());
        offscreenRenderpass.Destroy(device.Get());
        renderPass.Destroy(device.Get());
        multisamplingBuffer.Destroy(device.Get());
        graphicsPipelineForMesh.DestroyPipelineObjects(device.Get());
        graphicsPipelineForCubemapObjects.DestroyPipelineObjects(device.Get());
        graphicsPipelineForRigidBodyMesh.DestroyPipelineObjects(device.Get());
        graphicsPipelineForShadowMap.DestroyPipelineObjects(device.Get());


        for (size_t i = 0; i < drawCommandBuffer.size(); i++)
        {
            drawCommandPool[i].Destroy(device.Get());
        }
       

        commandPool.Destroy(device.Get());
        swapchain.DestroySwapchainObjects(device.Get());
        surface.Destroy(instance.get());
        device.Destroy();
        instance.destroy();
    }

}