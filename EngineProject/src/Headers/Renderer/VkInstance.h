#ifndef instance_h
#define instance_h

#include	"../../vendor/volk.h"
#include	"../Globals.h"
#include	"DataTypes.h"
#include	<spdlog/spdlog.h>
#include	<vector>
#include	<iostream>

#ifdef _DEBUG
static bool enableValidationLayer = true;
#else
static bool enableValidationLayer = false;
#endif

namespace Engine {
    //Слой валидации (проверка объектов при создании или удалении в соответствии со спецификацией KHRONOS)//
    static std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };

    //Функция определяющая как выводить и тип выводимых отладочных сообщений//
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
            //spdlog::info(pCallbackData->pMessage);
            //Globals::gLogger->info(pCallbackData->pMessage);
        }
        else {
            spdlog::warn(pCallbackData->pMessage);
            Globals::gLogger->warn(pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    //Сеанс Vulkan "Instance"//
    class Instance {
        VkInstance instance;
        //Информация о сеансе//
        VkApplicationInfo appInfo{};
        //Информация о создании сеанса//
        VkInstanceCreateInfo createInfo{};
        //Обработчик отладочных сообщений//
        VkDebugUtilsMessengerEXT debugMessenger; \
            //Информация о создании обработчика отладочных сообщений//
            VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
    public:
        //Создать сеанс//
        void createInstance() {
            //Инициализация библиотеки для загрузки процедур из vulkan-1.dll//
            if (volkInitialize() != VK_SUCCESS) {
                throw std::runtime_error("Failed to init volk");
            }

            //Информация о приложении//
            appInfo.apiVersion = VK_MAKE_VERSION(1, 2, 0);
            appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
            appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
            appInfo.pApplicationName = "Engine";
            appInfo.pEngineName = "Lava";
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

            std::vector<const char*> extensions;

            //Расширение для использования окна Win32//
            //в качестве  поверхности для вывода//
            extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

            //Расширение для использования поверхностей вывода//
            extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

            //Расширение для использования утилит отладки//
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            createInfo.enabledExtensionCount = uint32_t(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            if (enableValidationLayer) {
                createInfo.enabledLayerCount = (uint32_t)layers.size();
                createInfo.ppEnabledLayerNames = layers.data();
            }

            //Заполнение структуры для создания обработчика отладочных сообщений//
            messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            messengerInfo.pfnUserCallback = debugCallback;
            messengerInfo.pUserData = nullptr;
            messengerInfo.flags = 0;

            //Привязка структуры messengerInfo к структуре createInfo//
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengerInfo;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

            //Создание сеанса//
            EngineExec(
                vkCreateInstance(&createInfo, nullptr, &instance),
                "Instance"
            );

            //Загрузка сеанса в библиотеку volk//
            volkLoadInstance(instance);

            //Создание обработчика отладочных сообщений//
            EngineExec(
                vkCreateDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, &debugMessenger),
                "Debug Utils"
            );
        }

        VkInstance get() {
            return instance;
        }

        void destroy() {
            vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
            vkDestroyInstance(instance, nullptr);
        };
    };

    //namespace Globals{
    //	extern Instance gInstance;
    //}
}

#endif