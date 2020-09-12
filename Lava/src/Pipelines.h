#pragma once
#include "../vendor/volk.h"
#include "DescriptorSetLayouts.h"
#include "DataTypes.h"
#include <vector>
#include <iostream>
#include <fstream>
namespace Lava{
	class RenderPass{
		VkRenderPass renderPass;
	public:
		VkRenderPass getRenderPass() {
			return renderPass;
		}
		void createRenderPass(VkDevice device, VkSwapchainCreateInfoKHR swapchainCreateInfo, VkFormat depthImageFormat) {
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = swapchainCreateInfo.imageFormat;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.samples = MSAAsamples;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = depthImageFormat;
			depthAttachment.samples = MSAAsamples;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription resolveColorAttachment{};
			resolveColorAttachment.format = swapchainCreateInfo.imageFormat;
			resolveColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			resolveColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			resolveColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			resolveColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			resolveColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			resolveColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			resolveColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkAttachmentReference colorAttachmentReference{};
			colorAttachmentReference.attachment = (uint32_t)0;
			colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentReference{};
			depthAttachmentReference.attachment = (uint32_t)1;
			depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference resolveColorAttachmentReference{};
			resolveColorAttachmentReference.attachment = (uint32_t)2;
			resolveColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			if (MSAAsamples == VK_SAMPLE_COUNT_1_BIT) {
				resolveColorAttachmentReference.attachment = (uint32_t)VK_ATTACHMENT_UNUSED;
				colorAttachmentReference.attachment = (uint32_t)2;
			}
			

			VkSubpassDescription subpassDescription{};
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = (uint32_t)1;
			subpassDescription.pColorAttachments = &colorAttachmentReference;
			subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
			subpassDescription.pResolveAttachments = &resolveColorAttachmentReference;
	
			VkAttachmentDescription attachments[] = {
				colorAttachment,depthAttachment,resolveColorAttachment
			};

			VkRenderPassCreateInfo renderPassCreateInfo{};
			renderPassCreateInfo.pAttachments = attachments;
			renderPassCreateInfo.attachmentCount = (uint32_t)3;
			renderPassCreateInfo.pSubpasses = &subpassDescription;
			renderPassCreateInfo.subpassCount = (uint32_t)1;
			renderPassCreateInfo.dependencyCount = 1;
			renderPassCreateInfo.pDependencies = &dependency;
			renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create render pass");
			}
		}
		void destroy(VkDevice device){
			vkDestroyRenderPass(device,renderPass,nullptr);
		}
	};

	class GraphicsPipeline {
	protected:
		VkPipeline graphicsPipeline;
		VkGraphicsPipelineCreateInfo createInfo{};
		VkPipelineLayout pipelineLayout;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

	public:
		VkPipeline* pGetGraphicsPipeline() {
			return &graphicsPipeline;
		}
		VkPipeline get() {
			return graphicsPipeline;
		}
		VkPipelineLayout getPipelineLayout() {
			return pipelineLayout;
		}
		VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code) {
			VkShaderModule shaderModule;
			VkShaderModuleCreateInfo shaderModuleInfo = {};
			shaderModuleInfo.codeSize = code.size();
			shaderModuleInfo.pCode = (uint32_t*)code.data();
			shaderModuleInfo.sType = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

			VkResult result = vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &shaderModule);
			if (result != VK_SUCCESS) {
				std::cout << "Result: " << result << std::endl;;
				throw std::runtime_error("Failed to create shader module.");
			}

			return shaderModule;

		}
		std::vector<char> readShader(std::string path) {
			std::ifstream file(path, std::ios::ate | std::ios::binary);
			if (!file.is_open()) {
				throw std::runtime_error("Shader source file not found");
			}
			uint32_t size = (uint32_t)file.tellg();
			std::vector<char> shader(size);
			file.seekg(0);
			file.read(shader.data(), size);
			file.close();
			return shader;
		}
	};

	class GraphicsPipelineForGameObjects : public GraphicsPipeline {
		
	public:
		void createGraphicsPipeline(
			VkDevice device, 
			VkSwapchainCreateInfoKHR swapchainCreateInfo,
			VkDescriptorSetLayout setLayoutForGameObjects, 
			VkRenderPass renderPass
		) {
		
			std::vector<char> vertexShader = this->readShader("src/SPIRV/gameObjectVertex.spv");
			std::vector<char> fragmentShader = this->readShader("src/SPIRV/gameObjectFragment.spv");

			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
			VkVertexInputBindingDescription bindingDescription;
			bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VkPipelineDepthStencilStateCreateInfo depthState{};
			depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthState.depthTestEnable = VK_TRUE;
			depthState.depthWriteEnable = VK_TRUE;
			depthState.depthCompareOp = VK_COMPARE_OP_LESS;
			depthState.depthBoundsTestEnable = VK_FALSE;

			VkVertexInputAttributeDescription attribDescription{};
			attribDescription.binding = 0;
			attribDescription.location = 0;
			attribDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attribDescription.offset = offsetof(Vertex, pos);
			attributeDescriptions.push_back(attribDescription);
			attribDescription.binding = 0;
			attribDescription.location = 1;
			attribDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attribDescription.offset = offsetof(Vertex, color);
			attributeDescriptions.push_back(attribDescription);
			attribDescription.binding = 0;
			attribDescription.location = 2;
			attribDescription.format = VK_FORMAT_R32G32_SFLOAT;
			attribDescription.offset = offsetof(Vertex, texCoord);
			attributeDescriptions.push_back(attribDescription);
			attribDescription.binding = 0;
			attribDescription.location = 3;
			attribDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attribDescription.offset = offsetof(Vertex, normals);
			attributeDescriptions.push_back(attribDescription);

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
			vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)1;

			VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {};
			assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			assemblyInfo.primitiveRestartEnable = VK_FALSE;

			VkShaderModule vertexModule = CreateShaderModule(device, vertexShader);
			VkShaderModule fragmentModule = CreateShaderModule(device, fragmentShader);

			VkPipelineShaderStageCreateInfo vertexStage = {};
			vertexStage.module = vertexModule;
			vertexStage.sType = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertexStage.pName = "main";
			vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

			VkPipelineShaderStageCreateInfo fragmentStage = {};
			fragmentStage.module = fragmentModule;
			fragmentStage.sType = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragmentStage.pName = "main";

			VkPipelineShaderStageCreateInfo stages[] = { vertexStage,fragmentStage };

			VkViewport viewport = {};
			viewport.x = 0;
			viewport.y = 0;
			viewport.height = (float)swapchainCreateInfo.imageExtent.height;
			viewport.width = (float)swapchainCreateInfo.imageExtent.width;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissors = {};
			scissors.extent = swapchainCreateInfo.imageExtent;
			scissors.offset = {};

			VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportStateCreateInfo.pViewports = &viewport;
			viewportStateCreateInfo.viewportCount = 1;
			viewportStateCreateInfo.pScissors = &scissors;
			viewportStateCreateInfo.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
			rasterizationInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
			rasterizationInfo.depthBiasEnable = VK_FALSE;
			rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationInfo.lineWidth = 1.0f;
			rasterizationInfo.depthClampEnable = VK_FALSE;
			rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		
			VkPipelineMultisampleStateCreateInfo multisampleInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
			multisampleInfo.rasterizationSamples = MSAAsamples;
			multisampleInfo.minSampleShading = 1.0f;
			multisampleInfo.pSampleMask = nullptr;
			multisampleInfo.sampleShadingEnable = VK_FALSE;
			multisampleInfo.alphaToCoverageEnable = VK_FALSE;
			multisampleInfo.alphaToOneEnable = VK_FALSE;
			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
			colorBlendInfo.pAttachments = &colorBlendAttachment;
			colorBlendInfo.attachmentCount = 1;

			VkDynamicState dynamicStates{
				VK_DYNAMIC_STATE_VIEWPORT
			};

			VkPipelineDynamicStateCreateInfo dynamicStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			dynamicStateInfo.dynamicStateCount = 1;
			dynamicStateInfo.pDynamicStates = &dynamicStates;

			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.pSetLayouts = &setLayoutForGameObjects;
			pipelineLayoutInfo.setLayoutCount = 1;

			if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout");
			}

			createInfo.sType = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
			createInfo.pViewportState = &viewportStateCreateInfo;
			createInfo.pColorBlendState = &colorBlendInfo;
			createInfo.pMultisampleState = &multisampleInfo;
			createInfo.pRasterizationState = &rasterizationInfo;
			createInfo.renderPass = renderPass;
			createInfo.stageCount = 2;
			createInfo.pStages = stages;
			createInfo.pInputAssemblyState = &assemblyInfo;
			createInfo.subpass = 0;
			createInfo.layout = pipelineLayout;
			createInfo.pVertexInputState = &vertexInputInfo;
			createInfo.pDepthStencilState = &depthState;

			if (vkCreateGraphicsPipelines(device, 0, 1, &createInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create graphics pipeline");
			}
			vkDestroyShaderModule(device, vertexModule, nullptr);
			vkDestroyShaderModule(device, fragmentModule, nullptr);
		};
		void destroyPipelineObjects(VkDevice device) {
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			vkDestroyPipeline(device, graphicsPipeline, nullptr);
		}
	};

	class GraphicsPipelineForPointLightObjects :public GraphicsPipeline {
	public:
		void createGraphicsPipeline(
			VkDevice device, 
			VkSwapchainCreateInfoKHR swapchainCreateInfo, 
			VkDescriptorSetLayout setLayoutForPointLightObjects, 
			VkRenderPass renderPass
		) {
			std::vector<char> vertexShader = this->readShader("src/SPIRV/lightObjectVertex.spv");
			std::vector<char> fragmentShader = this->readShader("src/SPIRV/lightObjectFragment.spv");

			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
			VkVertexInputBindingDescription bindingDescription;
			bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VkPipelineDepthStencilStateCreateInfo depthState{};
			depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthState.depthTestEnable = VK_TRUE;
			depthState.depthWriteEnable = VK_TRUE;
			depthState.depthCompareOp = VK_COMPARE_OP_LESS;
			depthState.depthBoundsTestEnable = VK_FALSE;

			VkVertexInputAttributeDescription attribDescription{};
			attribDescription.binding = 0;
			attribDescription.location = 0;
			attribDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attribDescription.offset = offsetof(Vertex, pos);
			attributeDescriptions.push_back(attribDescription);
			attribDescription.binding = 0;
			attribDescription.location = 1;
			attribDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attribDescription.offset = offsetof(Vertex, color);
			attributeDescriptions.push_back(attribDescription);
			attribDescription.binding = 0;
			attribDescription.location = 2;
			attribDescription.format = VK_FORMAT_R32G32_SFLOAT;
			attribDescription.offset = offsetof(Vertex, texCoord);
			attributeDescriptions.push_back(attribDescription);
			attribDescription.binding = 0;
			attribDescription.location = 3;
			attribDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attribDescription.offset = offsetof(Vertex, normals);
			attributeDescriptions.push_back(attribDescription);

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
			vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)1;

			VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {};
			assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			assemblyInfo.primitiveRestartEnable = VK_FALSE;

			VkShaderModule vertexModule = CreateShaderModule(device, vertexShader);
			VkShaderModule fragmentModule = CreateShaderModule(device, fragmentShader);

			VkPipelineShaderStageCreateInfo vertexStage = {};
			vertexStage.module = vertexModule;
			vertexStage.sType = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertexStage.pName = "main";
			vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

			VkPipelineShaderStageCreateInfo fragmentStage = {};
			fragmentStage.module = fragmentModule;
			fragmentStage.sType = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragmentStage.pName = "main";

			VkPipelineShaderStageCreateInfo stages[] = { vertexStage,fragmentStage };

			VkViewport viewport = {};
			viewport.x = 0;
			viewport.y = 0;
			viewport.height = (float)swapchainCreateInfo.imageExtent.height;
			viewport.width = (float)swapchainCreateInfo.imageExtent.width;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissors = {};
			scissors.extent = swapchainCreateInfo.imageExtent;
			scissors.offset = {};

			VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportStateCreateInfo.pViewports = &viewport;
			viewportStateCreateInfo.viewportCount = 1;
			viewportStateCreateInfo.pScissors = &scissors;
			viewportStateCreateInfo.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
			rasterizationInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
			rasterizationInfo.depthBiasEnable = VK_FALSE;
			rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationInfo.lineWidth = 1.0f;
			rasterizationInfo.depthClampEnable = VK_FALSE;
			rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			VkPipelineMultisampleStateCreateInfo multisampleInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
			multisampleInfo.rasterizationSamples = MSAAsamples;
			multisampleInfo.minSampleShading = 1.0f;
			multisampleInfo.pSampleMask = nullptr;
			multisampleInfo.sampleShadingEnable = VK_FALSE;
			multisampleInfo.alphaToCoverageEnable = VK_FALSE;
			multisampleInfo.alphaToOneEnable = VK_FALSE;
			

			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
			colorBlendInfo.pAttachments = &colorBlendAttachment;
			colorBlendInfo.attachmentCount = 1;

			VkDynamicState dynamicStates{
				VK_DYNAMIC_STATE_VIEWPORT
			};

			VkPipelineDynamicStateCreateInfo dynamicStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			dynamicStateInfo.dynamicStateCount = 1;
			dynamicStateInfo.pDynamicStates = &dynamicStates;

			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.pSetLayouts = &setLayoutForPointLightObjects;
			pipelineLayoutInfo.setLayoutCount = 1;

			if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout");
			}

			createInfo.sType = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
			createInfo.pViewportState = &viewportStateCreateInfo;
			createInfo.pColorBlendState = &colorBlendInfo;
			createInfo.pMultisampleState = &multisampleInfo;
			createInfo.pRasterizationState = &rasterizationInfo;
			createInfo.renderPass = renderPass;
			createInfo.stageCount = 2;
			createInfo.pStages = stages;
			createInfo.pInputAssemblyState = &assemblyInfo;
			createInfo.subpass = 0;
			createInfo.layout = pipelineLayout;
			createInfo.pVertexInputState = &vertexInputInfo;
			createInfo.pDepthStencilState = &depthState;

			if (vkCreateGraphicsPipelines(device, 0, 1, &createInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create graphics pipeline");
			}
			vkDestroyShaderModule(device, vertexModule, nullptr);
			vkDestroyShaderModule(device, fragmentModule, nullptr);
		};
		void destroyPipelineObjects(VkDevice device) {
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			vkDestroyPipeline(device, graphicsPipeline, nullptr);
		}
	};

	class GraphicsPipelineForCubemapObjects : public GraphicsPipeline {

	public:
		void createGraphicsPipeline(
			VkDevice device,
			VkSwapchainCreateInfoKHR swapchainCreateInfo,
			VkDescriptorSetLayout setLayoutForCubemapObjects,
			VkRenderPass renderPass
		) {

			std::vector<char> vertexShader = this->readShader("src/SPIRV/skyboxVertex.spv");
			std::vector<char> fragmentShader = this->readShader("src/SPIRV/skyboxFragment.spv");

			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
			VkVertexInputBindingDescription bindingDescription;
			bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(CubemapVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VkPipelineDepthStencilStateCreateInfo depthState{};
			depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthState.depthTestEnable = VK_FALSE;
			depthState.depthWriteEnable = VK_FALSE;
			depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthState.depthBoundsTestEnable = VK_FALSE;

			VkVertexInputAttributeDescription attribDescription{};
			attribDescription.binding = 0;
			attribDescription.location = 0;
			attribDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attribDescription.offset = offsetof(CubemapVertex, pos);
			attributeDescriptions.push_back(attribDescription);
			
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
			vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)1;

			VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {};
			assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			assemblyInfo.primitiveRestartEnable = VK_FALSE;

			VkShaderModule vertexModule = CreateShaderModule(device, vertexShader);
			VkShaderModule fragmentModule = CreateShaderModule(device, fragmentShader);

			VkPipelineShaderStageCreateInfo vertexStage = {};
			vertexStage.module = vertexModule;
			vertexStage.sType = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertexStage.pName = "main";
			vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

			VkPipelineShaderStageCreateInfo fragmentStage = {};
			fragmentStage.module = fragmentModule;
			fragmentStage.sType = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragmentStage.pName = "main";

			VkPipelineShaderStageCreateInfo stages[] = { vertexStage,fragmentStage };

			VkViewport viewport = {};
			viewport.x = 0;
			viewport.y = 0;
			viewport.height = (float)swapchainCreateInfo.imageExtent.height;
			viewport.width = (float)swapchainCreateInfo.imageExtent.width;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissors = {};
			scissors.extent = swapchainCreateInfo.imageExtent;
			scissors.offset = {};

			VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportStateCreateInfo.pViewports = &viewport;
			viewportStateCreateInfo.viewportCount = 1;
			viewportStateCreateInfo.pScissors = &scissors;
			viewportStateCreateInfo.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
			rasterizationInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
			rasterizationInfo.depthBiasEnable = VK_FALSE;
			rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationInfo.lineWidth = 1.0f;
			rasterizationInfo.depthClampEnable = VK_FALSE;
			rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			VkPipelineMultisampleStateCreateInfo multisampleInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
			multisampleInfo.rasterizationSamples = MSAAsamples;
			multisampleInfo.minSampleShading = 1.0f;
			multisampleInfo.pSampleMask = nullptr;
			multisampleInfo.sampleShadingEnable = VK_FALSE;
			multisampleInfo.alphaToCoverageEnable = VK_FALSE;
			multisampleInfo.alphaToOneEnable = VK_FALSE;
			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
			colorBlendInfo.pAttachments = &colorBlendAttachment;
			colorBlendInfo.attachmentCount = 1;

			VkDynamicState dynamicStates{
				VK_DYNAMIC_STATE_VIEWPORT
			};

			VkPipelineDynamicStateCreateInfo dynamicStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			dynamicStateInfo.dynamicStateCount = 1;
			dynamicStateInfo.pDynamicStates = &dynamicStates;

			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.pSetLayouts = &setLayoutForCubemapObjects;
			pipelineLayoutInfo.setLayoutCount = 1;

			if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create pipeline layout");
			}

			createInfo.sType = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
			createInfo.pViewportState = &viewportStateCreateInfo;
			createInfo.pColorBlendState = &colorBlendInfo;
			createInfo.pMultisampleState = &multisampleInfo;
			createInfo.pRasterizationState = &rasterizationInfo;
			createInfo.renderPass = renderPass;
			createInfo.stageCount = 2;
			createInfo.pStages = stages;
			createInfo.pInputAssemblyState = &assemblyInfo;
			createInfo.subpass = 0;
			createInfo.layout = pipelineLayout;
			createInfo.pVertexInputState = &vertexInputInfo;
			createInfo.pDepthStencilState = &depthState;

			if (vkCreateGraphicsPipelines(device, 0, 1, &createInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create graphics pipeline");
			}
			vkDestroyShaderModule(device, vertexModule, nullptr);
			vkDestroyShaderModule(device, fragmentModule, nullptr);
		};
		void destroyPipelineObjects(VkDevice device) {
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			vkDestroyPipeline(device, graphicsPipeline, nullptr);
		}
	};
}
