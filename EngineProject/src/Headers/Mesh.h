#ifndef mesh_h
#define mesh_h

#include "Transform.h"
#include "Pipelines.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DescriptorPools.h"
#include "Swapchain.h"
#include "DescriptorSetLayouts.h"
#include "../../vendor/volk.h"
#include <vector>
#include "DataTypes.h"
#include "Buffers.h"
#include "Camera.h"
#include "CommandPool.h"
#include "Images.h"
#include "../../vendor/tiny_obj_loader.h"
namespace Engine{
	class Mesh {
	private:
		std::string										MeshPath;
		std::vector<DataTypes::MeshVertex_t>			Vertices;
		std::vector<uint32_t>						    Indexes;
		std::unordered_map<DataTypes::MeshVertex_t, uint32_t> UniqueVertices{};
		Texture											mTexture;
		VulkanBuffers::VertexBuffer					    VertexBuffer;
		VulkanBuffers::IndexBuffer						IndexBuffer;
		DataTypes::MVP_t								MVP{};
		DataTypes::Material_t							Material{};
		std::vector<VkDescriptorSet>				    DescriptorSets;
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersMVP;
		std::vector<VulkanBuffers::UniformBuffer>   	UniformBuffersSpotLightAttributes;
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersDebugCameraPos;
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersMaterial;
	
		void LoadModel(std::string modelPath);

		void CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForGameObjects, 
			VkDescriptorPool descriptorPoolForGameObjects, std::vector<VkImageView> swapchainImageViews);

		public:

		VkBuffer GetVertexBuffer();

		VkBuffer GetIndexBuffer();

		std::string pGetMeshPath();

		std::vector<DataTypes::MeshVertex_t>* GetVertices();

		std::vector<uint32_t> GetIndexes();

		DataTypes::Material_t GetMaterial();

		void SetBaseColorTexture(std::string path);

		void SetMaterial(DataTypes::Material_t mat);
	
		void Draw(VkCommandBuffer commandBuffer, int imageIndex);

		void CreateMesh(std::string modelPath);

		void UpdateUniforms(uint32_t imageIndex, VkDevice device, glm::mat4 TransformMatrixProduct, 
			std::vector<DataTypes::SpotlightAttributes_t*> spotlightAttributes);

		void Destroy();

	};

	class WireframeMesh {
		private:
		std::vector<DataTypes::WireframeMeshVertex_t> Vertices;
		std::vector<uint32_t>					Indexes;
		std::unordered_map<DataTypes::WireframeMeshVertex_t, uint32_t> UniqueVertices{};

		VulkanBuffers::VertexBuffer				VertexBuffer;
		VulkanBuffers::IndexBuffer				IndexBuffer;

		DataTypes::MVP_t MVP{};

		std::vector<VkDescriptorSet>			DescriptorSets;

		std::vector<VulkanBuffers::UniformBuffer> UniformBuffersMVP;

		void LoadModel(std::string modelPath, glm::vec3 color);

		void CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForRigidBodyMesh, 
			VkDescriptorPool descriptorPoolForRigidBodyMesh, std::vector<VkImageView> imageViews);

	public:

		WireframeMesh();

		Transform Transform;

		void Draw(VkCommandBuffer commandBuffer, int imageIndex);

		VkBuffer GetVertexBuffer();

		VkBuffer GetIndexBuffer();

		std::vector<DataTypes::WireframeMeshVertex_t> GetVertices();

		std::vector<uint32_t> GetIndexes();

		void CreateMesh(std::string modelPath, glm::vec3 color);

		void UpdateUniforms(uint32_t imageIndex, VkDevice device);

		void Destroy();

	};

	class CubemapMesh{
	private:
		std::vector<DataTypes::MeshVertex_t>  Vertices;
		std::vector<uint32_t>					  Indexes;
		std::vector<glm::vec3>					  CubeMapVertices;
		VulkanBuffers::VertexBuffer				  mVertexBuffer;
		VulkanBuffers::IndexBuffer			      mIndexBuffer;
		std::vector<VulkanBuffers::UniformBuffer> UniformBuffersVP;
		std::vector<VkDescriptorSet>			  DescriptorSets;
		CubemapTexture							  mCubemapTexture;
		

		void CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForCubemap, 
			VkDescriptorPool descriptorPoolForCubemap, std::vector<VkImageView> imageViews);

		void LoadModel(std::string modelPath);

		
	public:
		CubemapMesh();

		void CreateCubemapMesh(std::vector<std::string> paths);

		void Draw(VkCommandBuffer commandBuffer, int imageIndex);

		void UpdateUniforms(uint32_t imageIndex, VkDevice device);

		void Destroy();

	};
}


#endif