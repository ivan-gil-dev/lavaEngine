#ifndef device_h
#define device_h

#include	"../../vendor/volk.h"
#include	"DataTypes.h"
#include	<vector>
#include	<iostream>
namespace Engine {
    static std::vector<const char*> Layers = { "VK_LAYER_KHRONOS_validation" };

    //Расширение для использования swapchain (цепь изображений)//
    static std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    //Логическое устройство//
    class Device {
        VkDevice LogicalDevice = VK_NULL_HANDLE;
        VkDeviceCreateInfo CreateInfo{};
        //Очередь для выполнения команд//
        VkQueue GraphicsQueue = VK_NULL_HANDLE;;
    public:
        //Создать логическое устройство//
        void CreateDevice(VkPhysicalDevice device, DataTypes::QueueIndices_t indices) {
            float priority = 1.0f;

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

            //Передача номера очереди, поддерживающей команды отрисовки//
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.queueCount = 1;
            queueInfo.queueFamilyIndex = indices.graphicsQueueIndex; //Индекс очереди, поддерживающий команды отрисовки
            queueInfo.pQueuePriorities = &priority;
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos.push_back(queueInfo);

            //Дополнительные функции физического устройства//
            VkPhysicalDeviceFeatures features{};
            features.samplerAnisotropy = VK_TRUE;
            features.imageCubeArray = VK_TRUE;
            features.fillModeNonSolid = VK_TRUE;
            features.sampleRateShading = VK_TRUE;

            CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            CreateInfo.pEnabledFeatures = &features;
            CreateInfo.enabledLayerCount = (uint32_t)Layers.size();//Слой для отладки//
            CreateInfo.ppEnabledLayerNames = Layers.data();
            CreateInfo.pQueueCreateInfos = queueCreateInfos.data();
            CreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
            CreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();//Расширения//
            CreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (vkCreateDevice(device, &CreateInfo, nullptr, &LogicalDevice) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create logical device");
            }

            //Получение очереди из графического устройства//
            vkGetDeviceQueue(LogicalDevice, indices.graphicsQueueIndex, 0, &GraphicsQueue);
        }

        VkQueue GetGraphicsQueue() {
            return GraphicsQueue;
        }

        VkDevice Get() {
            return LogicalDevice;
        }

        void Destroy() {
            vkDestroyDevice(LogicalDevice, nullptr);
        }

        VkDevice* PGet() {
            return &LogicalDevice;
        }
    };

    //namespace Globals{
    //	extern Device gDevice;
    //}
    //
}

#endif 