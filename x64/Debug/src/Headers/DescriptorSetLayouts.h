#pragma once
#include	"../vendor/volk.h"
#include	<iostream>
namespace Lava{
	/*
		—хема задателей дескрипторов позвол€ет
		задать идентификаторы наборам задателей дескрипторов в соответствии
		с юниформами в шейдере, а также их типы
	*/

	class DescriptorSetLayout {
		protected:
		VkDescriptorSetLayout SetLayout;
		VkDescriptorSetLayoutCreateInfo CreateInfo{};

		public:
		VkDescriptorSetLayout* PGet() {
			return &SetLayout;
		}
		void Destroy( VkDevice device) {
			vkDestroyDescriptorSetLayout(device, SetLayout, nullptr);
		}
		virtual void CreateDescriptorSetLayout(VkDevice device) = 0;
	};

	class DescriptorSetLayoutForMesh : public DescriptorSetLayout {
		public:
		void CreateDescriptorSetLayout(VkDevice device) {

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

			VkDescriptorSetLayoutBinding debugCameraBinding{};
			debugCameraBinding.binding = 3;
			debugCameraBinding.descriptorCount = 1;
			debugCameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			debugCameraBinding.pImmutableSamplers = nullptr;
			debugCameraBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(debugCameraBinding);

			VkDescriptorSetLayoutBinding materialBinding{};
			materialBinding.binding = 4;
			materialBinding.descriptorCount = 1;
			materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			materialBinding.pImmutableSamplers = nullptr;
			materialBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(materialBinding);

			CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			CreateInfo.pBindings = setLayoutBindings.data();
			CreateInfo.bindingCount = (uint32_t)setLayoutBindings.size();
			if (vkCreateDescriptorSetLayout(device, &CreateInfo, nullptr, &SetLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor set layout");
			}
		}
	}gSetLayoutForMesh;

	class DescriptorSetLayoutForRigidBodyMesh : public DescriptorSetLayout {
		public:
		void CreateDescriptorSetLayout(VkDevice device) {

			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

			VkDescriptorSetLayoutBinding mvpBinding{};
			mvpBinding.binding = 0;
			mvpBinding.descriptorCount = 1;
			mvpBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			mvpBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			setLayoutBindings.push_back(mvpBinding);

			CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			CreateInfo.pBindings = setLayoutBindings.data();
			CreateInfo.bindingCount = (uint32_t)setLayoutBindings.size();
			if (vkCreateDescriptorSetLayout(device, &CreateInfo, nullptr, &SetLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor set layout");
			}
		}
	}gSetLayoutForRigidBodyMesh;

	class DescriptorSetLayoutForCubemapObjects : public DescriptorSetLayout {
		public:
		void CreateDescriptorSetLayout(VkDevice device) {
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

			CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			CreateInfo.pBindings = setLayoutBindings.data();
			CreateInfo.bindingCount = (uint32_t)setLayoutBindings.size();

			if (vkCreateDescriptorSetLayout(device, &CreateInfo, nullptr, &SetLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor set layout");
			}
		}
	}gSetLayoutForCubemapObjects;

}
