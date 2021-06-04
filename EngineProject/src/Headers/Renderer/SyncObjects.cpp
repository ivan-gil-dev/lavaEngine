#include "SyncObjects.h"

void Engine::SyncObjects::CreateSyncObjects(VkDevice device, int frameCount)
{
    ImageRenderedSemaphores.resize(frameCount);
    ImageAvailableSemaphores.resize(frameCount);
    Fences.resize(frameCount);
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < ImageRenderedSemaphores.size(); i++) {
        if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &ImageRenderedSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceCreateInfo, nullptr, &Fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create sync object");
        }
    }
}

std::vector<VkSemaphore> Engine::SyncObjects::GetImageRenderedSemaphores()
{
    return ImageRenderedSemaphores;
}

std::vector<VkSemaphore> Engine::SyncObjects::GetImageAvailableSemaphores()
{
    return ImageAvailableSemaphores;
}

std::vector<VkFence> Engine::SyncObjects::GetFences()
{
    return Fences;
}

void Engine::SyncObjects::DestroySyncObjects(VkDevice device)
{
    for (size_t i = 0; i < ImageRenderedSemaphores.size(); i++) {
        vkDestroySemaphore(device, ImageRenderedSemaphores[i], nullptr);
        vkDestroySemaphore(device, ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, Fences[i], nullptr);
    }
    ImageRenderedSemaphores.clear();
    ImageAvailableSemaphores.clear();
    Fences.clear();
}