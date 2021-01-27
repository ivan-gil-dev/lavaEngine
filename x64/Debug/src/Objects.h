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
	enum ObjectType{
		LAVA_OBJECT_TYPE_ENTITY,
		LAVA_OBJECT_TYPE_GAME_OBJECT,
		LAVA_OBJECT_TYPE_CUBEMAP_OBJECT
	};

	class Entity {
		protected :
		ObjectType type;
		int id;
		std::string name;
		
		public :
		Transform transform;
	
		Entity(){
			id = 0;
			name = "Entity";
			type = LAVA_OBJECT_TYPE_ENTITY;
		}

		void setName(std::string Name) {
			name = Name;
		}

		ObjectType getType(){
			return type;
		}

		void setId(int Id) {
			id = Id;
		}

		std::string getName(){
			return name;
		}

		int getId(){
			return id;
		}
	};

	class GameObject : public Entity{
		RigidBody rigidBody;
		Mesh mesh;
		public:
		Mesh* getMesh(){
			return &mesh;
		}

		RigidBody* getRigidBody(){
			return &rigidBody;
		}

		GameObject() {
			type = LAVA_OBJECT_TYPE_GAME_OBJECT;
			name = "Game Object";
			id = 0;
		}

		void updateUniformBuffers(uint32_t imageIndex, VkDevice device, VkExtent2D extent){
			getMesh()->updateUniformBuffers(imageIndex,  device,  extent);
			getRigidBody()->getDebugMesh()->updateUniformBuffers(imageIndex, device, extent);
		}

		void recordDrawCalls(VkCommandBuffer commandBuffer, int imageIndex){
			getMesh()->recordDrawCalls(commandBuffer,imageIndex);
			getRigidBody()->getDebugMesh()->recordDrawCalls(commandBuffer, imageIndex);
		}

		void destroy() {
			getMesh()->destroy();
			rigidBody.destroy(dynamicsWorld);
		}	

		void syncMeshTransformWithRigidBody(){
			if (getRigidBody()->getBulletRigidBody() != nullptr && getRigidBody()->getBulletRigidBody()->getMass() != 0 && getMesh()->getVertexBuffer() != VK_NULL_HANDLE) {
				
				btTransform worldTransform;
				worldTransform = getRigidBody()->getBulletRigidBody()->getWorldTransform();

				glm::vec3 position = glm::vec3(worldTransform.getOrigin().getX(),
					worldTransform.getOrigin().getY(),
					worldTransform.getOrigin().getZ());

				getMesh()->transform.translate(glm::vec3(position.x, position.y, position.z));

				glm::quat quaternion = glm::quat(
					(float)worldTransform.getRotation().getX(),
					(float)worldTransform.getRotation().getY(),
					(float)worldTransform.getRotation().getZ(),
					(float)worldTransform.getRotation().getW()
				);
			
				getMesh()->transform.setQuaternion(quaternion);
			}

			if (getRigidBody()->getDebugMesh()->getVertexBuffer() != VK_NULL_HANDLE && getRigidBody()->getBulletRigidBody()->getMass() != 0) {
				btTransform worldTransform;
				worldTransform = getRigidBody()->getBulletRigidBody()->getWorldTransform();

				glm::vec3 position = glm::vec3(worldTransform.getOrigin().getX(),
					worldTransform.getOrigin().getY(),
					worldTransform.getOrigin().getZ());

				getRigidBody()->getDebugMesh()->transform.translate(glm::vec3(position.x, position.y, position.z));

				glm::quat quaternion = glm::quat(
					(float)worldTransform.getRotation().getX(),
					(float)worldTransform.getRotation().getY(),
					(float)worldTransform.getRotation().getZ(),
					(float)worldTransform.getRotation().getW()
				);

				getRigidBody()->getDebugMesh()->transform.setQuaternion(quaternion);
			}

		}
	};

	class CubemapObject : public Entity{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indexes;

		std::vector<glm::vec3> cubeMapVertices;
		std::vector<UniformBuffer> uniformBuffersMp;

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

		std::vector<VkDescriptorSet> descriptorSets;

		CubemapTexture cubemapTexture;
		void CreateDescriptorSets(
			VkDevice device,
			VkDescriptorSetLayout descriptorSetLayoutForCubemap,
			VkDescriptorPool descriptorPoolForCubemap,
			std::vector<VkImageView> imageViews
		) {
			std::vector<VkDescriptorSetLayout> layouts(imageViews.size(), descriptorSetLayoutForCubemap);

			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = descriptorPoolForCubemap;
			allocateInfo.descriptorSetCount = (uint32_t)layouts.size();
			allocateInfo.pSetLayouts = layouts.data();

			descriptorSets.resize(layouts.size());

			VkResult result = vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets.data());
			if (result != VK_SUCCESS) {
				std::cout << result << std::endl;
				throw std::runtime_error("Failed to allocate descriptor sets");
			}

			std::vector<VkWriteDescriptorSet> writeDescriptorSets;
			for (size_t i = 0; i < descriptorSets.size(); i++) {
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffersMp[i].get();
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformObjectCubemapMp);

				VkWriteDescriptorSet mvpWriteDescriptorSet{};
				mvpWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				mvpWriteDescriptorSet.descriptorCount = 1;
				mvpWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				mvpWriteDescriptorSet.dstSet = descriptorSets[i];
				mvpWriteDescriptorSet.dstBinding = 0;
				mvpWriteDescriptorSet.dstArrayElement = 0;
				mvpWriteDescriptorSet.pBufferInfo = &bufferInfo;
				writeDescriptorSets.push_back(mvpWriteDescriptorSet);

				VkDescriptorImageInfo textureInfo{};
				textureInfo.imageView = cubemapTexture.getImageView();
				textureInfo.sampler = cubemapTexture.getImageSampler();
				textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkWriteDescriptorSet textureWriteDescriptorSet{};
				textureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				textureWriteDescriptorSet.descriptorCount = 1;
				textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				textureWriteDescriptorSet.dstSet = descriptorSets[i];
				textureWriteDescriptorSet.dstBinding = 1;
				textureWriteDescriptorSet.dstArrayElement = 0;
				textureWriteDescriptorSet.pImageInfo = &textureInfo;
				textureWriteDescriptorSet.pBufferInfo = &bufferInfo;
				writeDescriptorSets.push_back(textureWriteDescriptorSet);


				vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
				writeDescriptorSets.resize(0);
			}
		}
		void loadModel(std::string modelPath) {
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

						Vertex vertex{};
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
						vertices.push_back(vertex);
						indexes.push_back((uint32_t)indexes.size());
					}
					offset += shapes[i].mesh.num_face_vertices[j];
				}

			}
		}
		public:
		void init(
			std::vector<std::string> paths
		){
			type = LAVA_OBJECT_TYPE_CUBEMAP_OBJECT;
			name = "CubemapObect";
			id = 0;
			cubemapTexture.createCubemapTexture(
				device.get(),
				physicalDevice.get(),
				commandPool.get(),
				device.getGraphicsQueue(),
				paths
			);

			uniformBuffersMp.resize(swapchain.pGetImageViews()->size());

			for (size_t i = 0; i < swapchain.pGetImageViews()->size(); i++) {
				uniformBuffersMp[i].CreateUniformBuffer(
					physicalDevice.get(),
					device.get(),
					sizeof(UniformObjectCubemapMp)
				);
			}

			CreateDescriptorSets(
				device.get(),
				*setLayoutForCubemapObjects.pGet(),
				descriptorPoolForCubemapObjects.get(),
				*swapchain.pGetImageViews()
			);

			loadModel("assets/cube.obj");

			for (size_t i = 0; i < vertices.size(); i++) {
				cubeMapVertices.push_back(vertices[i].pos);
			}

			vertexBuffer.CreateVertexBuffer(physicalDevice.get(), device.get(), device.getGraphicsQueue(), commandPool.get(),
				cubeMapVertices.data(), sizeof(vertices[0].pos) * vertices.size());
			indexBuffer.CreateIndexBuffer(physicalDevice.get(), device.get(), device.getGraphicsQueue(), commandPool.get(),
				indexes.data(), sizeof(indexes[0]) * indexes.size());
		}
		
		void recordDrawCalls(VkCommandBuffer commandBuffer,  int imageIndex) {
			vkCmdBindPipeline(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				graphicsPipelineForCubemapObjects.get()
			);
			VkBuffer buffers[] = { vertexBuffer.get() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineForCubemapObjects.getPipelineLayout(),
				0, 1, &descriptorSets[imageIndex], 0, nullptr);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer.get(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, (uint32_t)indexes.size(), 1, 0, 0, 0);
		}
		
		void updateUniformBuffers(uint32_t imageIndex, VkDevice device, VkExtent2D extent) {
			UniformObjectCubemapMp cubemapMp;
			cubemapMp.model = camera.getCubemapView();
			cubemapMp.projection = camera.getProjectionMatrix();
			cubemapMp.model[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		
			void* data;
			vkMapMemory(device, uniformBuffersMp[imageIndex].getSize(), 0, sizeof(cubemapMp), 0, &data);
			memcpy(data, &cubemapMp, sizeof(cubemapMp));
			vkUnmapMemory(device, uniformBuffersMp[imageIndex].getSize());
		}

		void destroy(){
			cubemapTexture.destroyTexture(device.get());
			vkFreeDescriptorSets(device.get(), descriptorPoolForCubemapObjects.get(), (uint32_t)descriptorSets.size(), descriptorSets.data());
			for (size_t i = 0; i < uniformBuffersMp.size(); i++) {
				uniformBuffersMp[i].destroy(device.get());
			}
			vertexBuffer.destroy(device.get());
			indexBuffer.destroy(device.get());
		}
	
	};
}
