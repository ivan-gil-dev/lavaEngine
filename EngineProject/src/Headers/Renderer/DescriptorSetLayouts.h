#ifndef descriptorsetlayouts_h
#define descriptorsetlayouts_h

#include	"../../vendor/volk.h"
#include	"Images.h"
#include	<iostream>
#include	<vector>
namespace Engine{
	
	//VkDescriptorSetLayout задает параметры дескрипторов в наборе
	

	class DescriptorSetLayout {
		protected:
		VkDescriptorSetLayout SetLayout;
		VkDescriptorSetLayoutCreateInfo CreateInfo{};

		public:
		VkDescriptorSetLayout Get() {
			return SetLayout;
		}
        VkDescriptorSetLayout *pGet() {
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

            VkDescriptorSetLayoutBinding lightSpaceBinding{};
			lightSpaceBinding.binding = 8;
			lightSpaceBinding.descriptorCount = 1;
			lightSpaceBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			lightSpaceBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            setLayoutBindings.push_back(lightSpaceBinding);
			
			VkDescriptorSetLayoutBinding diffuseTexturesBinding{};
            diffuseTexturesBinding.binding = 1;
            diffuseTexturesBinding.descriptorCount = MAX_MATERIALS;
            diffuseTexturesBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            diffuseTexturesBinding.pImmutableSamplers = nullptr;
            diffuseTexturesBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            setLayoutBindings.push_back(diffuseTexturesBinding);
			
			VkDescriptorSetLayoutBinding spotlightBinding{};
			spotlightBinding.binding = 2;
			spotlightBinding.descriptorCount = 1;
			spotlightBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			spotlightBinding.pImmutableSamplers = nullptr;
			spotlightBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			setLayoutBindings.push_back(spotlightBinding);
			
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

            VkDescriptorSetLayoutBinding directionalLightBinding{};
			directionalLightBinding.binding = 5;
			directionalLightBinding.descriptorCount = 1;
			directionalLightBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			directionalLightBinding.pImmutableSamplers = nullptr;
			directionalLightBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            setLayoutBindings.push_back(directionalLightBinding);
			
            VkDescriptorSetLayoutBinding specularTexturesBinding{};
			specularTexturesBinding.binding = 6;
			specularTexturesBinding.descriptorCount = MAX_MATERIALS;
			specularTexturesBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			specularTexturesBinding.pImmutableSamplers = nullptr;
			specularTexturesBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            setLayoutBindings.push_back(specularTexturesBinding);

            VkDescriptorSetLayoutBinding shadowMapBinding{};
			shadowMapBinding.binding = 7;
			shadowMapBinding.descriptorCount = 1;
			shadowMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			shadowMapBinding.pImmutableSamplers = nullptr;
			shadowMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            setLayoutBindings.push_back(shadowMapBinding);

            VkDescriptorSetLayoutBinding roughnessMapBinding{};
			roughnessMapBinding.binding = 9;
			roughnessMapBinding.descriptorCount = MAX_MATERIALS;
			roughnessMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			roughnessMapBinding.pImmutableSamplers = nullptr;
			roughnessMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            setLayoutBindings.push_back(roughnessMapBinding);

            VkDescriptorSetLayoutBinding metallicMapBinding{};
			metallicMapBinding.binding = 10;
			metallicMapBinding.descriptorCount = MAX_MATERIALS;
			metallicMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			metallicMapBinding.pImmutableSamplers = nullptr;
			metallicMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            setLayoutBindings.push_back(metallicMapBinding);



			CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			CreateInfo.pBindings = setLayoutBindings.data();
			CreateInfo.bindingCount = (uint32_t)setLayoutBindings.size();
			
			
			if (vkCreateDescriptorSetLayout(device, &CreateInfo, nullptr, &SetLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create descriptor set layout");
			}
		}
	};

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
	};

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
	};

	class DescriptorSetLayoutForShadowMap : public DescriptorSetLayout{
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
	};


	/*namespace Globals{
		extern DescriptorSetLayoutForMesh gSetLayoutForMesh;
		extern DescriptorSetLayoutForRigidBodyMesh gSetLayoutForRigidBodyMesh;
		extern DescriptorSetLayoutForCubemapObjects gSetLayoutForCubemapObjects;
	}*/
	

}
#endif