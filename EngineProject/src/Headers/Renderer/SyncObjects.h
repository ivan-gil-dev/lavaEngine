#ifndef syncobjects_h
#define syncobjects_h

#include "../../vendor/volk.h"
#include "DataTypes.h"
#include <vector>

namespace Engine{
	class SyncObjects {
		std::vector<VkSemaphore> ImageRenderedSemaphores;
		std::vector<VkSemaphore> ImageAvailableSemaphores;
		std::vector<VkFence> Fences;
	public:
		void CreateSyncObjects(VkDevice device);

		std::vector<VkSemaphore> GetImageRenderedSemaphores();

		std::vector<VkSemaphore> GetImageAvailableSemaphores();

		std::vector<VkFence> GetFences();

		void DestroySyncObjects(VkDevice device);

	};

	//namespace Globals{
	//	extern SyncObjects gSyncObjects;
	//}
}

#endif