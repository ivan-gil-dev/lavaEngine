#ifndef commandpool_h
#define commandpool_h

#include	"../../vendor/volk.h"
#include	"DataTypes.h"
namespace Engine{

	
	//Пул для команд используется для выделения памяти под буферы с командами
	class CommandPool {
		VkCommandPool CommandPool; 
		public :

		void CreateCommandPool(DataTypes::QueueIndices_t indices, VkDevice device) {

			VkCommandPoolCreateInfo commandPoolCreateInfo{};{
				commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				commandPoolCreateInfo.queueFamilyIndex = indices.graphicsQueueIndex;
				commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			}
			
			if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &CommandPool) != VK_SUCCESS){
				throw std::runtime_error("Failed to create command pool");
			}
		}

		void Destroy(VkDevice device) {
			vkDestroyCommandPool(device, CommandPool, nullptr);
		}

		public:
		VkCommandPool Get() {
			return CommandPool;
		}

		VkCommandPool* PGet() {
			return &CommandPool;
		}

	};

}


#endif 