#pragma once
#include	"../vendor/volk.h"
#include	<GLFW/glfw3.h>
#include	<iostream>

namespace Lava{
	class Surface {
		VkSurfaceKHR surface;
		public:

		// В качестве поверхности для отрисовки используется окно GLFW
		void createSurface(GLFWwindow* window, VkInstance instance) {
			if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create surface");
			}
		}

		VkSurfaceKHR get() {
			return surface;
		}

		void destroy(VkInstance instance) {
			vkDestroySurfaceKHR(instance, surface, nullptr);
		}

	}surface;
}
