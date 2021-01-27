#pragma once
#include	"../vendor/volk.h"
#include	<iostream>
namespace Lava{
	class DescriptorSetLayout {
		protected:
		VkDescriptorSetLayout setLayout;
		VkDescriptorSetLayoutCreateInfo createInfo{};

		public:
		VkDescriptorSetLayout* pGet() {
			return &setLayout;
		}
		void destroy(VkDevice device) {
			vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
		}
	};

	class DescriptorSetLayoutForMesh : public DescriptorSetLayout {
		public:
		void createDescriptorSetLayoutForGameObjects(VkDevice device) {
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

			VkDescriptorSetLayoutBinding mvpBinding{};
			mvpBinding.binding = 0;
			mvpBinding.descriptorCount = 1;
			mvpBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			mvpBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			setLayoutBindings.push_back(mvpBinding);

			VkDescriptorSetLayoutBinding samplerBinding{};
			samplerBinding.binding = 1;
			samplerBinding.descriptorCount = 1;
			samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerBinding.pImmutableSamplers = nullptr;
			samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(samplerBinding);

			VkDescriptorSetLayoutBinding lightBinding{};
			lightBinding.binding = 2;
			lightBinding.descriptorCount = 1;
			lightBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			lightBinding.pImmutableSamplers = nullptr;
			lightBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(lightBinding);

			VkDescriptorSetLayoutBinding cameraBinding{};
			cameraBinding.binding = 3;
			cameraBinding.descriptorCount = 1;
			cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			cameraBinding.pImmutableSamplers = nullptr;
			cameraBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(cameraBinding);

			VkDescriptorSetLayoutBinding materialBinding{};
			materialBinding.binding = 4;
			materialBinding.descriptorCount = 1;
			materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			materialBinding.pImmutableSamplers = nullptr;
			materialBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(materialBinding);

			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.pBindings = setLayoutBindings.data();
			createInfo.bindingCount = (uint32_t)setLayoutBindings.size();
			if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &setLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor set layout");
			}
		}
	}setLayoutForMesh;

	class DescriptorSetLayoutForRigidBodyMesh : public DescriptorSetLayout {
		public:
		void createDescriptorSetLayoutForGameObjects(VkDevice device) {
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

			VkDescriptorSetLayoutBinding mvpBinding{};
			mvpBinding.binding = 0;
			mvpBinding.descriptorCount = 1;
			mvpBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			mvpBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			setLayoutBindings.push_back(mvpBinding);

			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.pBindings = setLayoutBindings.data();
			createInfo.bindingCount = (uint32_t)setLayoutBindings.size();
			if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &setLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor set layout");
			}
		}
	}setLayoutForRigidBodyMesh;

	class DescriptorSetLayoutForCubemapObjects : public DescriptorSetLayout {
		public:
		void createDescriptorSetLayoutForCubemapObjects(VkDevice device) {
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
			VkDescriptorSetLayoutBinding mvpBinding{};
			mvpBinding.binding = 0;
			mvpBinding.descriptorCount = 1;
			mvpBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			mvpBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			setLayoutBindings.push_back(mvpBinding);

			VkDescriptorSetLayoutBinding samplerBinding{};
			samplerBinding.binding = 1;
			samplerBinding.descriptorCount = 1;
			samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerBinding.pImmutableSamplers = nullptr;
			samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(samplerBinding);

			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.pBindings = setLayoutBindings.data();
			createInfo.bindingCount = (uint32_t)setLayoutBindings.size();
			if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &setLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor set layout");
			}
		}
	}setLayoutForCubemapObjects;

}
