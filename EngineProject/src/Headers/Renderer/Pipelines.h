#ifndef pipelines_h
#define pipelines_h

#include	"../../vendor/volk.h"
#include	"DescriptorSetLayouts.h"
#include	"DataTypes.h"
#include	<vector>
#include	<iostream>
#include	<fstream>

namespace Engine{
	static bool PBR = true;

	class RenderPass{
		VkRenderPass vRenderPass;
		public:
		VkRenderPass GetRenderPass();

		void CreateRenderPass(VkDevice device, VkSwapchainCreateInfoKHR swapchainCreateInfo, VkFormat depthImageFormat);

		void Destroy(VkDevice device);

	};

    class ShadowMapOffscreenRenderPass {
        VkRenderPass vRenderPass;
    public:
        VkRenderPass GetRenderPass();

		void CreateRenderPass(VkDevice device, VkSwapchainCreateInfoKHR swapchainCreateInfo, VkFormat depthImageFormat);

        void Destroy(VkDevice device);

    };

	class GraphicsPipeline {
		protected:
		VkPipeline vGraphicsPipeline;
		VkGraphicsPipelineCreateInfo CreateInfo{};
		VkPipelineLayout PipelineLayout;
		VkPipelineLayoutCreateInfo PipelineLayoutInfo{};

		public:
		VkPipeline* PGetGraphicsPipeline();

		VkPipeline Get();

		VkPipelineLayout GetPipelineLayout();

		VkShaderModule CreateShaderModule(VkDevice device, std::vector<char>& code);

		std::vector<char> ReadShader(std::string path);
	
		virtual void CreateGraphicsPipeline(VkDevice device, VkSwapchainCreateInfoKHR swapchainCreateInfo,
			VkDescriptorSetLayout setLayout, VkRenderPass renderPass) = 0;

		virtual void DestroyPipelineObjects(VkDevice device) = 0;
	};

	class GraphicsPipelineForMesh : public GraphicsPipeline {	
		public:
		void CreateGraphicsPipeline(VkDevice device, VkSwapchainCreateInfoKHR swapchainCreateInfo,
			VkDescriptorSetLayout setLayout, VkRenderPass renderPass);

		void DestroyPipelineObjects(VkDevice device);

	};

	class GraphicsPipelineForCubemapObjects : public GraphicsPipeline {
		public:
		void CreateGraphicsPipeline(VkDevice device, VkSwapchainCreateInfoKHR swapchainCreateInfo,
			VkDescriptorSetLayout setLayout, VkRenderPass renderPass);
		
		void DestroyPipelineObjects(VkDevice device);

	};

	class GraphicsPipelineForRigidBodyMesh : public GraphicsPipeline {
		public:
		void CreateGraphicsPipeline(VkDevice device, VkSwapchainCreateInfoKHR swapchainCreateInfo,
			VkDescriptorSetLayout setLayout, VkRenderPass renderPass);

		void DestroyPipelineObjects(VkDevice device);

	};

	class GraphicsPipelineForShadowMap : public GraphicsPipeline{

    public:
        void CreateGraphicsPipeline(VkDevice device, VkSwapchainCreateInfoKHR swapchainCreateInfo,
            VkDescriptorSetLayout setLayout, VkRenderPass renderPass);

        void DestroyPipelineObjects(VkDevice device);
	};
}

#endif