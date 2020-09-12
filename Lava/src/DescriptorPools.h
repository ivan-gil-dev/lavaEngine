#pragma once
#include "../vendor/volk.h"
#include <vector>
#include <iostream>
namespace Lava{
	class DescriptorPool {
	protected:
		VkDescriptorPool descriptorPool;
	public:

		VkDescriptorPool get() {
			return descriptorPool;
		}
		void destroy(VkDevice device) {
			vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		}
	};

	class DescriptorPoolForGameObjects : public DescriptorPool {
	public:
		void createDescriptorPoolforGameObjects(VkDevice device, std::vector<VkImageView> imageViews) {
			std::vector<VkDescriptorPoolSize> poolSizes;
			VkDescriptorPoolSize mvpPoolSize{};
			mvpPoolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			mvpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes.push_back(mvpPoolSize);

			VkDescriptorPoolSize textureSamplerPoolSize{};
			textureSamplerPoolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			textureSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes.push_back(textureSamplerPoolSize);

			VkDescriptorPoolSize pointLightAttributesPoolSize{};
			pointLightAttributesPoolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			pointLightAttributesPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes.push_back(pointLightAttributesPoolSize);

			VkDescriptorPoolSize cameraPosPoolSize{};
			cameraPosPoolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			cameraPosPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes.push_back(cameraPosPoolSize);

			VkDescriptorPoolSize materialPoolSize{};
			materialPoolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			materialPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes.push_back(materialPoolSize);
			

			VkDescriptorPoolCreateInfo createInfo{};
			createInfo.pPoolSizes = poolSizes.data();
			createInfo.poolSizeCount = (uint32_t)poolSizes.size();
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			createInfo.maxSets = (uint32_t)imageViews.size() * 1000;
			createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor pool");
			}
		}
	};

	class DescriptorPoolForPointLightObjects : public DescriptorPool {
	public:
		void createDescriptorPoolforPointLightObjects(VkDevice device, std::vector<VkImageView> imageViews) {
			std::vector<VkDescriptorPoolSize> poolSizes;
			VkDescriptorPoolSize poolSize{};
			poolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes.push_back(poolSize);
			poolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes.push_back(poolSize);
			VkDescriptorPoolSize textureSamplerPoolSize{};
			textureSamplerPoolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			textureSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes.push_back(textureSamplerPoolSize);


			VkDescriptorPoolCreateInfo createInfo{};
			createInfo.pPoolSizes = poolSizes.data();
			createInfo.poolSizeCount = (uint32_t)poolSizes.size();
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			createInfo.maxSets = (uint32_t)imageViews.size() * 1000;
			createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor pool");
			}
		}
	};

	class DescriptorPoolForCubemapObjects : public DescriptorPool {
	public:
		void createDescriptorPoolforCubemapObjects(VkDevice device, std::vector<VkImageView> imageViews) {
			std::vector<VkDescriptorPoolSize> poolSizes;
			VkDescriptorPoolSize poolSize{};
			poolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes.push_back(poolSize);
			VkDescriptorPoolSize textureSamplerPoolSize{};
			textureSamplerPoolSize.descriptorCount = (uint32_t)imageViews.size() * 1000;
			textureSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes.push_back(textureSamplerPoolSize);


			VkDescriptorPoolCreateInfo createInfo{};
			createInfo.pPoolSizes = poolSizes.data();
			createInfo.poolSizeCount = (uint32_t)poolSizes.size();
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			createInfo.maxSets = (uint32_t)imageViews.size() * 1000;
			createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor pool");
			}
		}
	};

	class DescriptorPoolForImgui : public DescriptorPool {
	public:
		void createDescriptorPoolforImgui(VkDevice device, std::vector<VkImageView> imageViews) {
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
			createInfo.pPoolSizes = poolSizes;
			createInfo.poolSizeCount = (uint32_t)11;
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			createInfo.maxSets = (uint32_t)imageViews.size() * 1000;
			createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor pool");
			}
		}
	};

}

