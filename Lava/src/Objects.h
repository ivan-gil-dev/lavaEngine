#pragma once
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_STATIC

#include "../vendor/volk.h"
#include "../vendor/glm/glm.hpp"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/gtc/quaternion.hpp"
#include "../vendor/glm/gtx/quaternion.hpp"
#include "../vendor/tiny_obj_loader.h"

#include "DescriptorSetLayouts.h"
#include "DataTypes.h"
#include "Images.h"
#include "Buffers.h"
#include "Camera.h"
#include "RigidBody.h"


#include <GLFW/glfw3.h>
#include <vector>

namespace Lava{
	class Mesh{
	private:

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indexes;
		
	public:
		virtual void loadModel(std::string modelPath){
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
		std::vector<Vertex> getVertices(){
			return vertices;
		}
		std::vector<uint32_t> getIndexes(){
			return indexes;
		}
	
	
	
	};

	class Entity {
	protected:
		int id;
		std::string name;

		glm::mat4 scaleMatrix{}, translationMatrix{}, rotationMatrix{};
		glm::vec3 position;

		glm::vec3 rotationAngles;
		glm::vec3 scaleValue;
		glm::quat quaternion;

		glm::vec3 initPosition;
		glm::vec3 initRotationAngles;

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
	public:
		void translate(glm::vec3 translation) {
			position = translation;
			translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
		}
		void setInitialTransform(){
			position = initPosition;
			rotationAngles = initRotationAngles;
			setRotation(rotationAngles);
			translate(position);
		}
		void setRotation(glm::vec3 Rotation){	
			    glm::quat quatX,quatY,quatZ;
				rotationAngles.x = Rotation.x;
				quatX = glm::angleAxis(glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			
				rotationAngles.y = Rotation.y;
				quatY = glm::angleAxis(glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			
				rotationAngles.z = Rotation.z;
				quatZ = glm::angleAxis(glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

				quaternion = quatZ * quatY * quatX;
				rotationMatrix = glm::toMat4(quaternion);
		}
		void scale(glm::vec3 ScaleValue) {
			scaleValue = ScaleValue;
			scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleValue.x, scaleValue.y, scaleValue.z));
		}
		void initTranslation(glm::vec3 translation){
			position = translation;
			initPosition = position;
			translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
		}
		void initRotation(glm::vec3 Rotation){
			glm::quat quatX, quatY, quatZ;
			rotationAngles.x = Rotation.x;
			quatX = glm::angleAxis(glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));

			rotationAngles.y = Rotation.y;
			quatY = glm::angleAxis(glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));

			rotationAngles.z = Rotation.z;
			quatZ = glm::angleAxis(glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			quaternion = quatZ * quatY * quatX;
			rotationMatrix = glm::toMat4(quaternion);
			initRotationAngles = rotationAngles;
		}
		VkBuffer getVertexBuffer() {
			return vertexBuffer.get();
		}
		VkBuffer getIndexBuffer() {
			return indexBuffer.get();
		}
		glm::vec3 getRotationAngles(){
			return rotationAngles;
		}
		glm::vec3 getPosition() {
			return position;
		}
		glm::vec3* pGetPosition() {
			return &position;
		}
		std::string getName(){
			return name;
		}
		int getId(){
			return id;
		}
	
	};

	class PointLightObject : public Entity {
		Texture texture;
		
		glm::vec3 lightColor;

		UniformObjectMVP ubo{};

		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<UniformBuffer> uniformBuffersMVP;
		std::vector<UniformBuffer> uniformBuffersPointLightColor;

		void CreateDescriptorSets(
			VkDevice device, 
			VkDescriptorSetLayout descriptorSetLayoutForPointLightObjects, 
			VkDescriptorPool descriptorPoolForPointLightObjects, 
			std::vector<VkImageView> imageViews
		) {

			std::vector<VkDescriptorSetLayout> layouts(imageViews.size(), descriptorSetLayoutForPointLightObjects);

			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = descriptorPoolForPointLightObjects;
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
				bufferInfo.buffer = uniformBuffersMVP[i].get();
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformObjectMVP);

				VkWriteDescriptorSet mvpWriteDescriptorSet{};
				mvpWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				mvpWriteDescriptorSet.descriptorCount = 1;
				mvpWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				mvpWriteDescriptorSet.dstSet = descriptorSets[i];
				mvpWriteDescriptorSet.dstBinding = 0;
				mvpWriteDescriptorSet.dstArrayElement = 0;
				mvpWriteDescriptorSet.pBufferInfo = &bufferInfo;
				writeDescriptorSets.push_back(mvpWriteDescriptorSet);

				VkDescriptorBufferInfo bufferInfo2{};
				bufferInfo2.buffer = uniformBuffersPointLightColor[i].get();
				bufferInfo2.offset = 0;
				bufferInfo2.range = sizeof(UniformObjectLightColor);

				VkWriteDescriptorSet lightColorWriteDescriptorSet{};
				lightColorWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				lightColorWriteDescriptorSet.descriptorCount = 1;
				lightColorWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				lightColorWriteDescriptorSet.dstSet = descriptorSets[i];
				lightColorWriteDescriptorSet.dstBinding = 2;
				lightColorWriteDescriptorSet.dstArrayElement = 0;
				lightColorWriteDescriptorSet.pBufferInfo = &bufferInfo2;
				writeDescriptorSets.push_back(lightColorWriteDescriptorSet);

				VkDescriptorImageInfo textureInfo{};
				textureInfo.imageView = texture.getImageView();
				textureInfo.sampler = texture.getImageSampler();
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

				//VkWriteDescriptorSet writeDescriptorSets[] = {
				//	mvpWriteDescriptorSet,lightColorWriteDescriptorSet
				//};

				vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
				writeDescriptorSets.resize(0);
			}
		}

		
	public:
		Mesh mesh;
		
		glm::vec3 getLightColor() {
			return lightColor;
		}
		VkDescriptorSet getDescriptorSetByIndex(int index) {
			return descriptorSets[index];
		}
		VkDescriptorSet* pGetDescriptorSetByIndex(int index) {
			return &descriptorSets[index];
		}
		void init(
			VkPhysicalDevice physicalDevice, 
			VkDevice device, 
			VkQueue commandBufferQueue, 
			VkCommandPool commandPool, 
			std::vector<VkImageView> imageViews,
			VkDescriptorSetLayout descriptorSetLayoutForPointLightObjects,
			VkDescriptorPool descriptorPoolForPointLightObjects
		) {
			
			position = { 0.0f,0.0f,0.0f };
			scaleValue = { 1.0f,1.0f,1.0f };
			quaternion = { 0.0f,0.0f,0.0f,0.0f };
			scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleValue.x, scaleValue.y, scaleValue.z));
			rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
			lightColor = { 1.0f,1.0f,1.0f };
			mesh.loadModel("assets/cube.obj");
			texture.createTexture(physicalDevice,device,commandBufferQueue,commandPool,"assets/transparent.png");

			vertexBuffer.CreateVertexBuffer(
				physicalDevice, 
				device, 
				commandBufferQueue, 
				commandPool,
				mesh.getVertices().data(), 
				sizeof(mesh.getVertices()[0]) * mesh.getVertices().size()
			);

			indexBuffer.CreateIndexBuffer(
				physicalDevice, 
				device, 
				commandBufferQueue, 
				commandPool, 
				mesh.getIndexes().data(), 
				sizeof(mesh.getIndexes()[0]) * mesh.getIndexes().size()
			);

			uniformBuffersMVP.resize(imageViews.size());
			uniformBuffersPointLightColor.resize(imageViews.size());
			for (size_t i = 0; i < imageViews.size(); i++) {
				uniformBuffersMVP[i].CreateUniformBuffer(physicalDevice, device, sizeof(UniformObjectMVP));
			}
			for (size_t i = 0; i < imageViews.size(); i++) {
				uniformBuffersPointLightColor[i].CreateUniformBuffer(physicalDevice, device, sizeof(UniformObjectLightColor));
			}


			CreateDescriptorSets(device,descriptorSetLayoutForPointLightObjects,descriptorPoolForPointLightObjects, imageViews);
		}
		void destroy(VkDevice device, VkDescriptorPool descriptorPoolForPointLightObjects) {
			texture.destroyTexture(device);
			vkFreeDescriptorSets(device, descriptorPoolForPointLightObjects, (uint32_t)descriptorSets.size(), descriptorSets.data());
			for (size_t i = 0; i < uniformBuffersMVP.size(); i++) {
				uniformBuffersMVP[i].destroy(device);
				uniformBuffersPointLightColor[i].destroy(device);

			}
			vertexBuffer.destroy(device);
			indexBuffer.destroy(device);

		}
		void updateUniformBuffers(GLFWwindow* window, uint32_t imageIndex, VkDevice device, VkExtent2D extent) {
			rotationMatrix = glm::toMat4(quaternion);
			ubo.model =   translationMatrix*rotationMatrix * scaleMatrix;

			//camera.processInput(window);

			ubo.view = camera.getView();
			ubo.proj = camera.getProjectionMatrix();
			ubo.proj[1][1] *= -1;

			void* data;
			vkMapMemory(device, uniformBuffersMVP[imageIndex].getSize(), 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, uniformBuffersMVP[imageIndex].getSize());

			UniformObjectLightColor mat;
			mat.lightColor = lightColor;
			vkMapMemory(device, uniformBuffersPointLightColor[imageIndex].getSize(), 0, sizeof(UniformObjectLightColor), 0, &data);
			memcpy(data, &mat, sizeof(UniformObjectLightColor));
			vkUnmapMemory(device, uniformBuffersPointLightColor[imageIndex].getSize());
		}
		void recordDrawCalls(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayoutForCubemapObjects, int imageIndex){
			VkBuffer buffers[] = { vertexBuffer.get() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutForCubemapObjects,
				0, 1, &descriptorSets[imageIndex], 0, nullptr);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer.get(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, (uint32_t)mesh.getIndexes().size(), 1, 0, 0, 0);
		}
	};

	class GameObject : public Entity {

		Texture texture;
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

		RigidBody rigidBody;
		
		Mesh mesh;

		UniformObjectMVP ubo{};
		UniformObjectMaterial material{};

		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<UniformBuffer> uniformBuffersMVP;
		std::vector<UniformBuffer> uniformBuffersPointLightAttributes;
		std::vector<UniformBuffer> uniformBuffersCameraPos;
		std::vector<UniformBuffer> uniformBuffersMaterial;

		void CreateDescriptorSets(
			VkDevice device, 
			VkDescriptorSetLayout descriptorSetLayoutForGameObjects, 
			VkDescriptorPool descriptorPoolForGameObjects, 
			std::vector<VkImageView> imageViews
		) {
			std::vector<VkDescriptorSetLayout> layouts(imageViews.size(), descriptorSetLayoutForGameObjects);

			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = descriptorPoolForGameObjects;
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
				bufferInfo.buffer = uniformBuffersMVP[i].get();
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformObjectMVP);

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
				textureInfo.imageView = texture.getImageView();
				textureInfo.sampler = texture.getImageSampler();
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

				VkDescriptorBufferInfo bufferInfo2{};
				bufferInfo2.buffer = uniformBuffersPointLightAttributes[i].get();
				bufferInfo2.offset = 0;
				bufferInfo2.range = sizeof(UniformObjectPointLightAttributes);

				VkWriteDescriptorSet pointLightAttributesWriteDescriptorSet{};
				pointLightAttributesWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				pointLightAttributesWriteDescriptorSet.descriptorCount = 1;
				pointLightAttributesWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				pointLightAttributesWriteDescriptorSet.dstSet = descriptorSets[i];
				pointLightAttributesWriteDescriptorSet.dstBinding = 2;
				pointLightAttributesWriteDescriptorSet.dstArrayElement = 0;
				pointLightAttributesWriteDescriptorSet.pBufferInfo = &bufferInfo2;
				writeDescriptorSets.push_back(pointLightAttributesWriteDescriptorSet);

				VkDescriptorBufferInfo bufferInfo3{};
				bufferInfo3.buffer = uniformBuffersCameraPos[i].get();
				bufferInfo3.offset = 0;
				bufferInfo3.range = sizeof(UniformObjectCameraPos);

				VkWriteDescriptorSet cameraPosWriteDescriptorSet{};
				cameraPosWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				cameraPosWriteDescriptorSet.descriptorCount = 1;
				cameraPosWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				cameraPosWriteDescriptorSet.dstSet = descriptorSets[i];
				cameraPosWriteDescriptorSet.dstBinding = 3;
				cameraPosWriteDescriptorSet.dstArrayElement = 0;
				cameraPosWriteDescriptorSet.pBufferInfo = &bufferInfo3;
				writeDescriptorSets.push_back(cameraPosWriteDescriptorSet);

				VkDescriptorBufferInfo bufferInfo4{};
				bufferInfo4.buffer = uniformBuffersMaterial[i].get();
				bufferInfo4.offset = 0;
				bufferInfo4.range = sizeof(UniformObjectMaterial);

				VkWriteDescriptorSet materialWriteDescriptorSet{};
				materialWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				materialWriteDescriptorSet.descriptorCount = 1;
				materialWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				materialWriteDescriptorSet.dstSet = descriptorSets[i];
				materialWriteDescriptorSet.dstBinding = 4;
				materialWriteDescriptorSet.dstArrayElement = 0;
				materialWriteDescriptorSet.pBufferInfo = &bufferInfo4;
				writeDescriptorSets.push_back(materialWriteDescriptorSet);

				vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
				writeDescriptorSets.resize(0);
			}
		}

	public:
		void createRigidBodyFromMesh(
			float mass,
			float restitution,
			float friction,
			btDynamicsWorld* dynamicsWorld
		){
			rigidBody.createShapeFromVertices(mesh.getVertices(),true);
			rigidBody.createBodyWithMass(
				mass,
				friction,
				restitution,
				dynamicsWorld,
				id
			);
		}
		
		void createRigidBodyPlane(
			float mass,
			float restitution,
			float friction,
			btDynamicsWorld* dynamicsWorld
			
		){
			rigidBody.createPlaneShape();
			rigidBody.createBodyWithMass(
				mass,
				friction,
				restitution,
				dynamicsWorld,
				id
			);
		
		}
		
		btRigidBody* getRigidBody(){
			if (rigidBody.getRigidBody()!=nullptr) {
				return rigidBody.getRigidBody();
			}else{
				return nullptr;
			}
		}
		
		UniformObjectMaterial getMaterial() {
			return material;
		}

		void setMaterial(UniformObjectMaterial mat) {
			material = mat;
		}

		void setName(std::string Name){
			name = Name;
		}

		void setId(int Id){
			id = Id;
		}

		VkDescriptorSet getDescriptorSetByIndex(int index) {
			return descriptorSets[index];
		}

		VkDescriptorSet* pGetDescriptorSetByIndex(int index) {
			return &descriptorSets[index];
		}

		GameObject() {
		}

		void setRigidBodyObjectTransform(){
			if (rigidBody.getRigidBody()!=nullptr) {
				btVector3 pos(position.x, position.y,position.z);
				btQuaternion rot;
				rot.setX(quaternion.x);
				rot.setY(quaternion.y);
				rot.setZ(quaternion.z);
				rot.setW(quaternion.w);
				btTransform worldTransform(rot, pos);
				rigidBody.getRigidBody()->setWorldTransform(worldTransform);
			}
		}

		void init(
			VkPhysicalDevice physicalDevice, 
			VkDevice device, 
			VkQueue commandBufferQueue, 
			VkCommandPool commandPool,
			std::vector<VkImageView> imageViews, 
			VkDescriptorSetLayout descriptorSetLayoutForGameObjects,
			VkDescriptorPool descriptorPoolForGameObjects,
			std::string modelPath, 
			std::string texturePath
		) {
			name = "Game Object";
			id = 0;

			position = { 0.0f,0.0f,0.0f };
			scaleValue = { 1.0f,1.0f,1.0f };
			quaternion = { 0.0f,0.0f,0.0f,0.0f };
			rotationAngles = {0.0f,0.0f,0.0f};
			material = { 0.1f,1.0f,5.0f };
			scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleValue.x, scaleValue.y, scaleValue.z));
			rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));

			mesh.loadModel(modelPath);
			texture.createTexture(physicalDevice, device, commandBufferQueue, commandPool, texturePath);

			vertexBuffer.CreateVertexBuffer(physicalDevice, device, commandBufferQueue, commandPool,
				mesh.getVertices().data(), sizeof(mesh.getVertices()[0]) * mesh.getVertices().size());
			indexBuffer.CreateIndexBuffer(physicalDevice, device, commandBufferQueue, commandPool,
				mesh.getIndexes().data(), sizeof(mesh.getIndexes()[0]) * mesh.getIndexes().size());

			uniformBuffersMVP.resize(imageViews.size());
			uniformBuffersPointLightAttributes.resize(imageViews.size());
			uniformBuffersCameraPos.resize(imageViews.size());
			uniformBuffersMaterial.resize(imageViews.size());

			for (size_t i = 0; i < imageViews.size(); i++) {
				uniformBuffersMVP[i].CreateUniformBuffer(physicalDevice, device, sizeof(UniformObjectMVP));
				uniformBuffersPointLightAttributes[i].CreateUniformBuffer(physicalDevice, device, sizeof(UniformObjectPointLightAttributes));
				uniformBuffersCameraPos[i].CreateUniformBuffer(physicalDevice, device, sizeof(UniformObjectCameraPos));
				uniformBuffersMaterial[i].CreateUniformBuffer(physicalDevice, device, sizeof(UniformObjectMaterial));
			}
			

			CreateDescriptorSets(device, descriptorSetLayoutForGameObjects,descriptorPoolForGameObjects, imageViews);
		}
		
		void destroy(VkDevice device, VkDescriptorPool descriptorPoolForGameObjects, btDynamicsWorld *dynamicsWorld) {
			texture.destroyTexture(device);
			vkFreeDescriptorSets(device, descriptorPoolForGameObjects, (uint32_t)descriptorSets.size(), descriptorSets.data());
			for (size_t i = 0; i < uniformBuffersMVP.size(); i++) {
				uniformBuffersMVP[i].destroy(device);
				uniformBuffersPointLightAttributes[i].destroy(device);
				uniformBuffersCameraPos[i].destroy(device);
				uniformBuffersMaterial[i].destroy(device);

			}
			vertexBuffer.destroy(device);
			indexBuffer.destroy(device);
			if (rigidBody.getRigidBody() != nullptr) {
				rigidBody.destroy(dynamicsWorld);
			}
		
		}	

		void updateUniformBuffers(
			GLFWwindow* window, 
			uint32_t imageIndex, 	  
			VkDevice device, 
			VkExtent2D extent, 
			PointLightObject lightObject
		) {

			rotationMatrix = glm::toMat4(quaternion);
			ubo.model =  translationMatrix * rotationMatrix * scaleMatrix;
			ubo.view = camera.getView();
			ubo.proj = camera.getProjectionMatrix();

			ubo.proj[1][1] *= -1;
			void* data;
			vkMapMemory(device, uniformBuffersMVP[imageIndex].getSize(), 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, uniformBuffersMVP[imageIndex].getSize());

			UniformObjectPointLightAttributes light;
			light.lightColor = lightObject.getLightColor();
			light.lightPosition = lightObject.getPosition();

			vkMapMemory(device, uniformBuffersPointLightAttributes[imageIndex].getSize(), 0, sizeof(UniformObjectPointLightAttributes), 0, &data);
			memcpy(data, &light, sizeof(UniformObjectPointLightAttributes));
			vkUnmapMemory(device, uniformBuffersPointLightAttributes[imageIndex].getSize());

			UniformObjectCameraPos cameraPos;
			cameraPos.pos = camera.getPosition();
			vkMapMemory(device, uniformBuffersCameraPos[imageIndex].getSize(), 0, sizeof(UniformObjectCameraPos), 0, &data);
			memcpy(data, &cameraPos, sizeof(UniformObjectCameraPos));
			vkUnmapMemory(device, uniformBuffersCameraPos[imageIndex].getSize());

			vkMapMemory(device, uniformBuffersMaterial[imageIndex].getSize(), 0, sizeof(UniformObjectMaterial), 0, &data);
			memcpy(data, &material, sizeof(UniformObjectMaterial));
			vkUnmapMemory(device, uniformBuffersMaterial[imageIndex].getSize());
		}
		
		void syncRigidBodyWithGraphics(){
			if (rigidBody.getRigidBody() != nullptr && rigidBody.getRigidBody()->getMass() != 0) {
				btTransform worldTransform;
				worldTransform = rigidBody.getRigidBody()->getInterpolationWorldTransform();

				position = glm::vec3(worldTransform.getOrigin().getX(),
					worldTransform.getOrigin().getY(),
					worldTransform.getOrigin().getZ());

				translationMatrix = glm::translate(
					glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));

				quaternion = glm::quat(
					(float)worldTransform.getRotation().getX(),
					(float)worldTransform.getRotation().getY(),
					(float)worldTransform.getRotation().getZ(),
					(float)worldTransform.getRotation().getW()
				);

				rigidBody.getShape()->setLocalScaling(btVector3(
					scaleValue.x,
					scaleValue.y,
					scaleValue.z
				));
			}
		}
		
		void recordDrawCalls(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayoutForGameObjects, int imageIndex) {
			VkBuffer buffers[] = { vertexBuffer.get() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutForGameObjects,
				0, 1, &descriptorSets[imageIndex], 0, nullptr);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer.get(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, (uint32_t)mesh.getIndexes().size(), 1, 0, 0, 0);
		}
	};

	class CubemapObject : public Entity{
		Mesh mesh;
		std::vector<glm::vec3> cubeMapVertices;
		std::vector<UniformBuffer> uniformBuffersMp;

		std::vector<VkDescriptorSet> descriptorSets;

		CubemapTexture cubemapTexture;
	public:
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

		void init(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkCommandPool commandPool,
			VkQueue commandBufferQueue,
			std::vector<std::string> paths,
			std::vector<VkImageView> imageViews,
			VkDescriptorSetLayout descriptorSetLayoutForCubemap,
			VkDescriptorPool descriptorPoolForCubemap
		){
			cubemapTexture.createCubemapTexture(
				device,
				physicalDevice,
				commandPool,
				commandBufferQueue,
				paths
			);

			uniformBuffersMp.resize(imageViews.size());

			for (size_t i = 0; i < imageViews.size(); i++) {
				uniformBuffersMp[i].CreateUniformBuffer(
					physicalDevice,
					device,
					sizeof(UniformObjectCubemapMp)
				);
			}

			CreateDescriptorSets(
				device,
				descriptorSetLayoutForCubemap,
				descriptorPoolForCubemap,
				imageViews
			);

			position = { 0.0f,0.0f,0.0f };
			scaleValue = { 1.0f,1.0f,1.0f };
			quaternion = { 0.0f,0.0f,0.0f,0.0f };

			mesh.loadModel("assets/cube.obj");

			for (size_t i = 0; i < mesh.getVertices().size(); i++) {
				cubeMapVertices.push_back(mesh.getVertices()[i].pos);
			}

			vertexBuffer.CreateVertexBuffer(physicalDevice, device, commandBufferQueue, commandPool,
				cubeMapVertices.data(), sizeof(mesh.getVertices()[0].pos) * mesh.getVertices().size());
			indexBuffer.CreateIndexBuffer(physicalDevice, device, commandBufferQueue, commandPool,
				mesh.getIndexes().data(), sizeof(mesh.getIndexes()[0]) * mesh.getIndexes().size());
		}
		
		void recordDrawCalls(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayoutForCubemap, int imageIndex) {
			VkBuffer buffers[] = { vertexBuffer.get() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutForCubemap,
				0, 1, &descriptorSets[imageIndex], 0, nullptr);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer.get(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, (uint32_t)mesh.getIndexes().size(), 1, 0, 0, 0);
		}
		
		void updateUniformBuffers(uint32_t imageIndex, VkDevice device, VkExtent2D extent) {
			rotationMatrix = glm::toMat4(quaternion);

			UniformObjectCubemapMp cubemapMp;
			cubemapMp.model = camera.getCubemapView();
			cubemapMp.projection = glm::perspective(glm::radians(80.0f), (extent.width / (float)extent.height), 0.1f, 256.0f);
			cubemapMp.model[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		

			void* data;
			vkMapMemory(device, uniformBuffersMp[imageIndex].getSize(), 0, sizeof(cubemapMp), 0, &data);
			memcpy(data, &cubemapMp, sizeof(cubemapMp));
			vkUnmapMemory(device, uniformBuffersMp[imageIndex].getSize());

	
		}
		
		void clear(VkDevice device, VkDescriptorPool descriptorPoolForCubemapObjects){
			cubemapTexture.destroyTexture(device);
			vkFreeDescriptorSets(device, descriptorPoolForCubemapObjects, (uint32_t)descriptorSets.size(), descriptorSets.data());
			for (size_t i = 0; i < uniformBuffersMp.size(); i++) {
				uniformBuffersMp[i].destroy(device);
			}
			vertexBuffer.destroy(device);
			indexBuffer.destroy(device);
		}
	
	};



} 
