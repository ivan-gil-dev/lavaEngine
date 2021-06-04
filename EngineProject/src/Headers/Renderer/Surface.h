#ifndef surface_h
#define surface_h

#include	"../../vendor/volk.h"
#include	"../Globals.h"
#include	<iostream>

namespace Engine {
    class Surface {
        VkSurfaceKHR vSurface;
    public:

        void CreateSurface(HWND hwnd, HINSTANCE hInstance, VkInstance instance) {
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.hinstance = hInstance;
            surfaceCreateInfo.hwnd = hwnd;
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

            if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &vSurface) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create Win32 surface");
            }
        }

        VkSurfaceKHR Get() {
            return vSurface;
        }

        void Destroy(VkInstance instance) {
            vkDestroySurfaceKHR(instance, vSurface, nullptr);
        }
    };

    /*namespace Globals{
        extern Surface gSurface;
    }*/
}

#endif