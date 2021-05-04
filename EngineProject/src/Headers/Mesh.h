#ifndef mesh_h
#define mesh_h

#include "Transform.h"
#include "../../vendor/volk.h"
#include <vector>
#include "Renderer/DataTypes.h"
#include "Renderer/Buffers.h"

#include "Renderer/Images.h"
#include "../../vendor/tiny_obj_loader.h"
namespace Engine{
	struct Face {
		std::vector<uint32_t> indexes;
		VulkanBuffers::IndexBuffer indexBuffer;
		short MatID;
		std::string diffuseMapPath;
		std::string specularMapPath;

	};

	class Mesh {
	private:
		std::string										MeshPath;

		bool											MaterialsFound;
		bool											IsCreated = false;

		std::vector<DataTypes::MeshVertex_t>			Vertices;
		std::vector<Face>								Faces;


		std::unordered_map<DataTypes::MeshVertex_t, uint32_t> UniqueVertices{};

		VulkanBuffers::VertexBuffer					    VertexBuffer;

		DataTypes::MVP_t								MVP{};

		DataTypes::Material_t							Material{};

		std::vector<VkDescriptorSet>				    DescriptorSets;
	private:
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersMVP_b0;

		std::vector<Texture>							DiffuseTextures_b1;
		std::vector<Texture>							SpecularTextures_b6;
		Texture											Blank;

		std::vector<VulkanBuffers::UniformBuffer>   	UniformBuffersSpotLightAttributes_b2;
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersDebugCameraPos_b3;
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersMaterial_b4;
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersDirectionalLightAttributes_b5;
	
		void LoadModel(std::string modelPath);

		void CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForGameObjects, 
			VkDescriptorPool descriptorPoolForGameObjects, std::vector<VkImageView> swapchainImageViews);

		public:
		bool IsMeshCreated() {
			return IsCreated;
		}

		bool IsMaterialsFound() {
				return MaterialsFound;
		}

		std::string pGetMeshPath();

		std::vector<DataTypes::MeshVertex_t>* GetVertices();

		DataTypes::Material_t GetMaterial();

		void SetMaterial(DataTypes::Material_t mat);
	
		void Draw(VkCommandBuffer commandBuffer, int imageIndex);

		void CreateMesh(std::string modelPath);

		void UpdateUniforms(uint32_t imageIndex, VkDevice device, glm::vec3 cameraPosition, DataTypes::ViewProjection_t viewProjection, glm::mat4 TransformMatrixProduct,
			std::vector<DataTypes::PointLightAttributes_t*> spotlightAttributes, std::vector <DataTypes::DirectionalLightAttributes_t*> directionalLightAttributes);

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

		std::vector<DataTypes::WireframeMeshVertex_t> GetVertices();

		void CreateMesh(std::string modelPath, glm::vec3 color);

		void UpdateUniforms(uint32_t imageIndex, VkDevice device, DataTypes::ViewProjection_t viewProjection);

		void Destroy();

	};

	class CubemapMesh{
	private:
		std::vector<DataTypes::MeshVertex_t>	  Vertices;
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

		void UpdateUniforms(uint32_t imageIndex, VkDevice device, DataTypes::ViewProjection_t viewProjection);

		void Destroy();

	};
}


#endif