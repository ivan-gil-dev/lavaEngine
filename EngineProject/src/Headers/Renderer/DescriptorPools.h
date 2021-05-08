#ifndef descriptorpools_h
#define descriptorpools_h


#include	"../../vendor/volk.h"
#include	<vector>
#include	<iostream>
#include	"Images.h"

namespace Engine{

	//Ïóë äåñêðèïòîðîâ çàäàåò ìàêñèìàëüíî äîñòóïíîå êîëè÷åñòâî äåñêðèïòîðîâ â êàæäîì íàáîðå
	
	class DescriptorPool {
		protected:
		VkDescriptorPool vDescriptorPool;
		public:
		VkDescriptorPool Get() {
			return vDescriptorPool;
		}

		void Destroy(VkDevice device) {
			vkDestroyDescriptorPool(device, vDescriptorPool, nullptr);
		}

		virtual void CreateDescriptorPool(VkDevice device, int swapchainImageViewCount) = 0;
	};

	class DescriptorPoolForMesh : public DescriptorPool {
		public:
		void CreateDescriptorPool(VkDevice device, int swapchainImageViewCount){

			std::vector<VkDescriptorPoolSize> poolSizes;
			VkDescriptorPoolSize mvpPoolSize{};
            mvpPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000;
            mvpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes.push_back(mvpPoolSize);
			
            VkDescriptorPoolSize lightSpacePoolSize{};
			lightSpacePoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000;
			lightSpacePoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes.push_back(lightSpacePoolSize);

			VkDescriptorPoolSize diffuseTextureSamplerPoolSize{};
            diffuseTextureSamplerPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000 * MAX_MATERIALS;
            diffuseTextureSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes.push_back(diffuseTextureSamplerPoolSize);
			
			VkDescriptorPoolSize pointLightAttributesPoolSize{};
            pointLightAttributesPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000 * MAX_SPOTLIGHTS;
            pointLightAttributesPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes.push_back(pointLightAttributesPoolSize);
			
			VkDescriptorPoolSize ñameraPosPoolSize{};
            ñameraPosPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000;
            ñameraPosPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes.push_back(ñameraPosPoolSize);
			
			VkDescriptorPoolSize materialPoolSize{};
            materialPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000;
            materialPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes.push_back(materialPoolSize);
			
            VkDescriptorPoolSize directionalLightAttributesPoolSize{};
			directionalLightAttributesPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000 * MAX_DLIGHTS;
			directionalLightAttributesPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes.push_back(directionalLightAttributesPoolSize);

            VkDescriptorPoolSize specularTextureSamplerPoolSize{};
			specularTextureSamplerPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000 * MAX_MATERIALS;
			specularTextureSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes.push_back(specularTextureSamplerPoolSize);

            VkDescriptorPoolSize shadowMapPoolSize{};
			shadowMapPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000;
			shadowMapPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes.push_back(shadowMapPoolSize);

			VkDescriptorPoolCreateInfo createInfo{};
            createInfo.pPoolSizes = poolSizes.data();
            createInfo.poolSizeCount = (uint32_t)poolSizes.size();
            createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            createInfo.maxSets = (uint32_t)swapchainImageViewCount * 1000;
            createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			
			
			if (vkCreateDescriptorPool(device, &createInfo, nullptr, &vDescriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor pool");
			}
		}

	};

	class DescriptorPoolForRigidBodyMesh : public DescriptorPool {
		public:
		void CreateDescriptorPool(VkDevice device, int swapchainImageViewCount) {
			std::vector<VkDescriptorPoolSize> poolSizes;

			VkDescriptorPoolSize mvpPoolSize{};
            mvpPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000;
            mvpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes.push_back(mvpPoolSize);
			
			VkDescriptorPoolCreateInfo createInfo{};
			createInfo.pPoolSizes = poolSizes.data();
			createInfo.poolSizeCount = (uint32_t)poolSizes.size();
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			createInfo.maxSets = (uint32_t)swapchainImageViewCount * 1000;
			createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			
			if (vkCreateDescriptorPool(device, &createInfo, nullptr, &vDescriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor pool");
			}
		}

	};

	class DescriptorPoolForCubemapObjects : public DescriptorPool {
		public:
			void CreateDescriptorPool(VkDevice device, int swapchainImageViewCount) {

			std::vector<VkDescriptorPoolSize> poolSizes;

			VkDescriptorPoolSize poolSize{};
            poolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000;
            poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes.push_back(poolSize);
			
			VkDescriptorPoolSize textureSamplerPoolSize{};
			//Òåêñòóðà skybox'à
            textureSamplerPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000;
            textureSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes.push_back(textureSamplerPoolSize);
			
			VkDescriptorPoolCreateInfo createInfo{};
            createInfo.pPoolSizes = poolSizes.data();
            createInfo.poolSizeCount = (uint32_t)poolSizes.size();
            createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            createInfo.maxSets = (uint32_t)swapchainImageViewCount * 1000;
            createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			
			if (vkCreateDescriptorPool(device, &createInfo, nullptr, &vDescriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor pool");
			}
		}

	};

	class DescriptorPoolForImgui : public DescriptorPool {
		public:
		void CreateDescriptorPool(VkDevice device, int swapchainImageViewCount) {
			VkDescriptorPoolSize poolSizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo createInfo{};
			//
			{
				createInfo.pPoolSizes = poolSizes;
				createInfo.poolSizeCount = (uint32_t)11;
				createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				createInfo.maxSets = (uint32_t)swapchainImageViewCount * 1000;
				createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			}
			

			if (vkCreateDescriptorPool(device, &createInfo, nullptr, &vDescriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor pool");
			}
		}

	};

	class DescriptorPoolForShadowMap : public DescriptorPool{
	public:
        void CreateDescriptorPool(VkDevice device, int swapchainImageViewCount) {
            std::vector<VkDescriptorPoolSize> poolSizes;

            VkDescriptorPoolSize mvpPoolSize{};
            mvpPoolSize.descriptorCount = (uint32_t)swapchainImageViewCount * 1000;
            mvpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes.push_back(mvpPoolSize);

            VkDescriptorPoolCreateInfo createInfo{};
            createInfo.pPoolSizes = poolSizes.data();
            createInfo.poolSizeCount = (uint32_t)poolSizes.size();
            createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            createInfo.maxSets = (uint32_t)swapchainImageViewCount * 1000;
            createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

            if (vkCreateDescriptorPool(device, &createInfo, nullptr, &vDescriptorPool) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor pool");
            }
        }
	};

}

#endif