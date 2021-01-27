#pragma once
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_STATIC

#include	"../vendor/volk.h"
#include	"../vendor/glm/glm.hpp"
#include	"../vendor/glm/gtc/matrix_transform.hpp"
#include	"../vendor/glm/gtc/quaternion.hpp"
#include	"../vendor/glm/gtx/quaternion.hpp"
#include	"../vendor/tiny_obj_loader.h"

#include	"DescriptorSetLayouts.h"
#include	"DataTypes.h"
#include	"Images.h"
#include	"Buffers.h"
#include	"Camera.h"
#include	"RigidBody.h"
#include	"Transform.h"
#include	"Mesh.h"

#include	<GLFW/glfw3.h>
#include	<vector>

namespace Lava{
	enum EntityType{
		LAVA_ENTITY_TYPE_GAME_OBJECT,
		LAVA_ENTITY_TYPE_CUBEMAP_OBJECT,
		LAVA_ENTITY_TYPE_SPOTLIGHT_OBJECT
	};

	class Entity {
		protected :
		EntityType Type;
		int ID;
		std::string Name;
		public :
		Transform Transform;

		virtual Mesh* GetMesh(){return nullptr;};

		virtual RigidBody* GetRigidBody(){return nullptr;};

		virtual void UpdateUniforms(uint32_t imageIndex, VkDevice device){};

		virtual void Draw(VkCommandBuffer commandBuffer, int imageIndex){};

		virtual void Destroy()=0;

		virtual void Start(){};
		
		virtual void Update(){};

		virtual void ApplyPhysicsToEntity(){};

		virtual void ApplyEntityTransformToRigidbodyAndMesh(){};

		Entity(){
			ID = 0;
			Name = "Entity";
		}

		void SetName(std::string name) {
			Name = name;
		}

		void SetId(int Id) {
			ID = Id;
		}

		EntityType GetType(){
			return Type;
		}

		std::string GetName(){
			return Name;
		}

		int GetId(){
			return ID;
		}
	};

	class SpotlightObject : public Entity {
	glm::vec3 LightColor;
	Mesh	  DebugMesh;
	public:
		SpotlightObject() {
			LightColor = glm::vec3(1.0f,1.0f,1.0f);
		}
	};

	class GameObject : public Entity{
		RigidBody mRigidBody;
		Mesh	  mMesh;
		public:
	    Mesh* GetMesh() override {
			return &mMesh;
		}

		RigidBody* GetRigidBody() override {
			return &mRigidBody;
		}

		GameObject() {
			Type = LAVA_ENTITY_TYPE_GAME_OBJECT;
			Name = "Game Object";
			ID = 0;
		}

		void UpdateUniforms(uint32_t imageIndex, VkDevice device) override{
			mRigidBody.GetDebugMesh()->UpdateUniforms(imageIndex, device);
			mMesh.UpdateUniforms(imageIndex, device);
		}

		void Draw(VkCommandBuffer commandBuffer, int imageIndex) override {
			if (LAVA_GLOBAL_ENABLE_RIGIDBODY_MESH) {
				mRigidBody.GetDebugMesh()->Draw(commandBuffer, imageIndex);
			}
			mMesh.Draw(commandBuffer,imageIndex);
		}

		void Destroy() override{
			GetMesh()->Destroy();
			mRigidBody.Destroy(gDynamicsWorld);
		}	

		void ApplyPhysicsToEntity() override{
			if (Type == LAVA_ENTITY_TYPE_GAME_OBJECT) {
				if (mRigidBody.IsCreated && mRigidBody.GetBulletRigidBody()->getMass() != 0) {

					btTransform worldTransform;
					worldTransform = mRigidBody.GetBulletRigidBody()->getWorldTransform();

					glm::vec3 position = glm::vec3(worldTransform.getOrigin().getX(),
												   worldTransform.getOrigin().getY(),
												   worldTransform.getOrigin().getZ());

					Transform.Translate(glm::vec3(position.x, position.y, position.z));

					glm::quat quaternion = glm::quat(
						(float)worldTransform.getRotation().getX(),
						(float)worldTransform.getRotation().getY(),
						(float)worldTransform.getRotation().getZ(),
						(float)worldTransform.getRotation().getW()
					);

					Transform.SetQuaternion(quaternion);

					if (mRigidBody.GetDebugMesh()->IsCreated) {
						mRigidBody.GetDebugMesh()->Transform = Transform;
					}
				}

				if (mMesh.IsCreated) {
					mMesh.vTransform = Transform;
				}
			}
		}
	
		void ApplyEntityTransformToRigidbodyAndMesh(){
			if (mMesh.IsCreated) {
				mMesh.vTransform = Transform;
			}
			if (mRigidBody.IsCreated) {
				mRigidBody.SetRigidBodyTransform(Transform);
			}
			
		}

	};

	class CubemapObject : public Entity{
		std::vector<DataTypes::Vertex>			  Vertices;
		std::vector<uint32_t>					  Indexes;
		std::vector<glm::vec3>					  CubeMapVertices;
		VulkanBuffers::VertexBuffer				  VertexBuffer;
		VulkanBuffers::IndexBuffer			      IndexBuffer;
		std::vector<VulkanBuffers::UniformBuffer> UniformBuffersMp;
		std::vector<VkDescriptorSet>			  DescriptorSets;
		CubemapTexture							  CubemapTexture;
		
		void CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForCubemap,
			VkDescriptorPool descriptorPoolForCubemap, std::vector<VkImageView> imageViews) {

			std::vector<VkDescriptorSetLayout> layouts(imageViews.size(), descriptorSetLayoutForCubemap);

			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = descriptorPoolForCubemap;
			allocateInfo.descriptorSetCount = (uint32_t)layouts.size();
			allocateInfo.pSetLayouts = layouts.data();

			DescriptorSets.resize(layouts.size());

			VkResult result = vkAllocateDescriptorSets(device, &allocateInfo, DescriptorSets.data());
			if (result != VK_SUCCESS) {
				std::cout << result << std::endl;
				throw std::runtime_error("Failed to allocate descriptor sets");
			}

			std::vector<VkWriteDescriptorSet> writeDescriptorSets;

			for (size_t i = 0; i < DescriptorSets.size(); i++) {
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = UniformBuffersMp[i].Get();
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(DataTypes::U_Struct_CubemapMV);

				VkWriteDescriptorSet mvpWriteDescriptorSet{};
				mvpWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				mvpWriteDescriptorSet.descriptorCount = 1;
				mvpWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				mvpWriteDescriptorSet.dstSet = DescriptorSets[i];
				mvpWriteDescriptorSet.dstBinding = 0;
				mvpWriteDescriptorSet.dstArrayElement = 0;
				mvpWriteDescriptorSet.pBufferInfo = &bufferInfo;
				writeDescriptorSets.push_back(mvpWriteDescriptorSet);

				VkDescriptorImageInfo textureInfo{};
				textureInfo.imageView = CubemapTexture.GetImageView();
				textureInfo.sampler =	CubemapTexture.GetImageSampler();
				textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet textureWriteDescriptorSet{};
				textureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				textureWriteDescriptorSet.descriptorCount = 1;
				textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				textureWriteDescriptorSet.dstSet = DescriptorSets[i];
				textureWriteDescriptorSet.dstBinding = 1;
				textureWriteDescriptorSet.dstArrayElement = 0;
				textureWriteDescriptorSet.pImageInfo = &textureInfo;
				textureWriteDescriptorSet.pBufferInfo = &bufferInfo;
				writeDescriptorSets.push_back(textureWriteDescriptorSet);


				vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
				writeDescriptorSets.resize(0);
			}
		}
		
		void LoadModel(std::string modelPath) {
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::material_t> materials;
			std::vector<tinyobj::shape_t> shapes;
			std::string warn, err;
			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
				throw std::runtime_error(warn + err);
			}
			for (size_t i = 0; i < shapes.size(); i++) {
				size_t offset = 0;
				for (size_t j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++) {
					for (size_t k = 0; k < shapes[i].mesh.num_face_vertices[j]; k++) {
						DataTypes::Vertex vertex{};
						vertex.pos = {
							attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 0],
							attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 1],
							attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 2]
						};

						vertex.texCoord = {
							attrib.texcoords[2 * shapes[i].mesh.indices[k + offset].texcoord_index + 0],
							1.0f - attrib.texcoords[2 * shapes[i].mesh.indices[k + offset].texcoord_index + 1]

						};
						vertex.color = { 1.0f,1.0f,1.0f };
						vertex.normals = {
							attrib.normals[3 * shapes[i].mesh.indices[k + offset].normal_index + 0],
							attrib.normals[3 * shapes[i].mesh.indices[k + offset].normal_index + 1],
							attrib.normals[3 * shapes[i].mesh.indices[k + offset].normal_index + 2]
						};
						Vertices.push_back(vertex);
						Indexes.push_back((uint32_t)Indexes.size());
					}
					offset += shapes[i].mesh.num_face_vertices[j];
				}
			}
		}
		public:
		void Init(std::vector<std::string> paths){
			Type = LAVA_ENTITY_TYPE_CUBEMAP_OBJECT;
			Name = "CubemapObect";
			ID = 0;

			CubemapTexture.CreateCubemapTexture(
				gDevice.Get(),
				gPhysicalDevice.Get(),
				gCommandPool.Get(),
				gDevice.GetGraphicsQueue(),
				paths
			);

			UniformBuffersMp.resize(gSwapchain.PGetImageViews()->size());

			for (size_t i = 0; i < gSwapchain.PGetImageViews()->size(); i++) {
				UniformBuffersMp[i].CreateUniformBuffer(
					gPhysicalDevice.Get(),
					gDevice.Get(),
					sizeof(DataTypes::U_Struct_CubemapMV)
				);
			}

			CreateDescriptorSets(
				gDevice.Get(),
				*gSetLayoutForCubemapObjects.PGet(),
				gDescriptorPoolForCubemapObjects._Get_(),
				*gSwapchain.PGetImageViews()
			);

			LoadModel("assets/cube.obj");

			for (size_t i = 0; i < Vertices.size(); i++) {
				CubeMapVertices.push_back(Vertices[i].pos);
			}

			VertexBuffer.CreateVertexBuffer(
				gPhysicalDevice.Get(), 
				gDevice.Get(),
				gDevice.GetGraphicsQueue(), 
				gCommandPool.Get(),
				CubeMapVertices.data(), 
				sizeof(Vertices[0].pos) * Vertices.size());

			IndexBuffer.CreateIndexBuffer(
				gPhysicalDevice.Get(), 
				gDevice.Get(), 
				gDevice.GetGraphicsQueue(), 
				gCommandPool.Get(),
				Indexes.data(), 
				sizeof(Indexes[0]) * Indexes.size());
		}
		
		void Draw(VkCommandBuffer commandBuffer, int imageIndex) {
			vkCmdBindPipeline(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				gGraphicsPipelineForCubemapObjects.Get()
			);

			if (LAVA_GLOBAL_ENABLE_DYNAMIC_VIEWPORT) {
				vkCmdSetViewport(commandBuffer, 0, 1, &gEditor3DViewport);
				vkCmdSetScissor(commandBuffer, 0, 1, &gEditor3DScissors);
			}

			VkBuffer buffers[] = { VertexBuffer.Get() };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gGraphicsPipelineForCubemapObjects.GetPipelineLayout(),
				0, 1, &DescriptorSets[imageIndex], 0, nullptr);
			vkCmdBindIndexBuffer(commandBuffer, IndexBuffer.Get(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, (uint32_t)Indexes.size(), 1, 0, 0, 0);

		}
		
		void UpdateUniforms(uint32_t imageIndex, VkDevice device) override{
			DataTypes::U_Struct_CubemapMV cubemapMp;
			cubemapMp.model = debugCamera.GetCubemapViewForVulkan();
			cubemapMp.view = debugCamera.GetProjectionMatrix();
			cubemapMp.model[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		
			void* data;
			vkMapMemory(device, UniformBuffersMp[imageIndex].GetDeviceMemory(), 0, sizeof(cubemapMp), 0, &data);
			memcpy(data, &cubemapMp, sizeof(cubemapMp));
			vkUnmapMemory(device, UniformBuffersMp[imageIndex].GetDeviceMemory());
		}

		void Destroy(){
			CubemapTexture.DestroyTexture(gDevice.Get());
			vkFreeDescriptorSets(gDevice.Get(),
								 gDescriptorPoolForCubemapObjects._Get_(),
								(uint32_t)DescriptorSets.size(), 
								 DescriptorSets.data());

			for (size_t i = 0; i < UniformBuffersMp.size(); i++) {
				UniformBuffersMp[i].Destroy(gDevice.Get());
			}

			VertexBuffer.Destroy(gDevice.Get());
			IndexBuffer.Destroy(gDevice.Get());
		}
	};

	
}
