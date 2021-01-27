#pragma once
#include	"Transform.h"
#include	"Pipelines.h"
#include	"Device.h"
#include	"PhysicalDevice.h"
#include	"DescriptorPools.h"
#include	"Swapchain.h"
#include	"DescriptorSetLayouts.h"
#include	"../vendor/volk.h"
#include	<vector>
#include	"DataTypes.h"
#include	"Buffers.h"
namespace Lava{
	class Mesh {
	private:
		bool isCreated = false;
		
		std::string meshPath;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indexes;
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		Texture texture;
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

		UniformObjectMVP ubo{};
		UniformObjectMaterial material{};

		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<UniformBuffer> uniformBuffersMVP;
		std::vector<UniformBuffer> uniformBuffersPointLightAttributes;
		std::vector<UniformBuffer> uniformBuffersCameraPos;
		std::vector<UniformBuffer> uniformBuffersMaterial;
		
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

						if (uniqueVertices.count(vertex)==0) {
							uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
							vertices.push_back(vertex);
						}

						indexes.push_back(uniqueVertices[vertex]);
						//indexes.push_back((uint32_t)indexes.size());
					}
					offset += shapes[i].mesh.num_face_vertices[j];
				}
			}
		}
		
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
		Transform transform;
		void recordDrawCalls(VkCommandBuffer commandBuffer, int imageIndex) {
			if (isCreated) {
				vkCmdBindPipeline(
					commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					graphicsPipelineForMesh.get()
				);
				VkBuffer buffers[] = { vertexBuffer.get() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineForMesh.getPipelineLayout(),
					0, 1, &descriptorSets[imageIndex], 0, nullptr);

				vkCmdBindIndexBuffer(commandBuffer, indexBuffer.get(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer, (uint32_t)indexes.size(), 1, 0, 0, 0);
			}
		}
		VkBuffer getVertexBuffer() {
			if (isCreated) {
				return vertexBuffer.get();
			}else{
				return VK_NULL_HANDLE;
			}
		}
		VkBuffer getIndexBuffer() {
			if (isCreated) {
				return indexBuffer.get();
			}else{
				return VK_NULL_HANDLE;
			}
		}
		VkDescriptorSet getDescriptorSetByIndex(int index) {
			if (isCreated) {
			
				return descriptorSets[index];
			}
			else {
				return VK_NULL_HANDLE;
			}
		}
		VkDescriptorSet* pGetDescriptorSetByIndex(int index) {
			if (isCreated) {
				return &descriptorSets[index];
			}
			else {
				return VK_NULL_HANDLE;
			}
		}
		std::vector<Vertex> *getVertices() {
			if (isCreated) {
				return &vertices;
			}else{
				return nullptr;
			}
		}
		std::vector<uint32_t> getIndexes() {
			if (isCreated) {
				return indexes;
			}
		}
		UniformObjectMaterial getMaterial() {
			if (isCreated) {
				return material;
			}
		}
		
		void addBaseColorTexture(std::string path){
			texture.destroyTexture(device.get());
			texture.createTexture(physicalDevice.get(), device.get(), device.getGraphicsQueue(), commandPool.get(), path);
			CreateDescriptorSets(device.get(), *setLayoutForMesh.pGet(), descriptorPoolForMesh.get(), *swapchain.pGetImageViews());
		}

		void setMaterial(UniformObjectMaterial mat) {
			if (isCreated) {
				material = mat;
			}
		}

		void createMesh(std::string modelPath) {
			material = { 0.1f,1.0f,5.0f };
			meshPath = modelPath;

			loadModel(modelPath);
			texture.createTexture(physicalDevice.get(), device.get(), device.getGraphicsQueue(), commandPool.get(), "");

			vertexBuffer.CreateVertexBuffer(physicalDevice.get(), device.get(), device.getGraphicsQueue(), commandPool.get(),
				vertices.data(), sizeof(vertices[0]) * vertices.size());
			indexBuffer.CreateIndexBuffer(physicalDevice.get(), device.get(), device.getGraphicsQueue(), commandPool.get(),
				indexes.data(), sizeof(indexes[0]) * indexes.size());

			uniformBuffersMVP.resize(swapchain.pGetImageViews()->size());
			uniformBuffersPointLightAttributes.resize(swapchain.pGetImageViews()->size());
			uniformBuffersCameraPos.resize(swapchain.pGetImageViews()->size());
			uniformBuffersMaterial.resize(swapchain.pGetImageViews()->size());

			for (size_t i = 0; i < swapchain.pGetImageViews()->size(); i++) {
				uniformBuffersMVP[i].CreateUniformBuffer(physicalDevice.get(), device.get(), sizeof(UniformObjectMVP));
				uniformBuffersPointLightAttributes[i].CreateUniformBuffer(physicalDevice.get(), device.get(), sizeof(UniformObjectPointLightAttributes));
				uniformBuffersCameraPos[i].CreateUniformBuffer(physicalDevice.get(), device.get(), sizeof(UniformObjectCameraPos));
				uniformBuffersMaterial[i].CreateUniformBuffer(physicalDevice.get(), device.get(), sizeof(UniformObjectMaterial));
			}

			CreateDescriptorSets(device.get(), *setLayoutForMesh.pGet(), descriptorPoolForMesh.get(), *swapchain.pGetImageViews());
			isCreated = true;
		}

		void updateUniformBuffers(uint32_t imageIndex, VkDevice device, VkExtent2D extent) {
			if (isCreated) {
				ubo.model = transform.getMatrixProduct();
				ubo.view = camera.getView();
				ubo.proj = camera.getProjectionMatrix();

				ubo.proj[1][1] *= -1;
				void* data;
				vkMapMemory(device, uniformBuffersMVP[imageIndex].getSize(), 0, sizeof(ubo), 0, &data);
				memcpy(data, &ubo, sizeof(ubo));
				vkUnmapMemory(device, uniformBuffersMVP[imageIndex].getSize());

				UniformObjectPointLightAttributes light;
				light.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
				light.lightPosition = glm::vec3(0.0f, 15.0f, 12.0f);

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
		}

		void destroy() {
			if (isCreated) {
				texture.destroyTexture(device.get());
				vkFreeDescriptorSets(device.get(), descriptorPoolForMesh.get(), (uint32_t)descriptorSets.size(), descriptorSets.data());
				for (size_t i = 0; i < uniformBuffersMVP.size(); i++) {
					uniformBuffersMVP[i].destroy(device.get());
					uniformBuffersPointLightAttributes[i].destroy(device.get());
					uniformBuffersCameraPos[i].destroy(device.get());
					uniformBuffersMaterial[i].destroy(device.get());

				}
				vertexBuffer.destroy(device.get());
				indexBuffer.destroy(device.get());
				isCreated = false;
			}
		}

		std::string getMeshPath(){
			return meshPath;
		}
		
	};

	class WireframeMesh {
		private:
		bool isCreated = false;

		std::vector<WireframeVertex> vertices;
		std::vector<uint32_t> indexes;
		std::unordered_map<WireframeVertex, uint32_t> uniqueVertices{};

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

		UniformObjectMVP ubo{};

		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<UniformBuffer> uniformBuffersMVP;

		void loadModel(std::string modelPath, glm::vec3 color) {
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

						WireframeVertex vertex{};
						vertex.pos = {
							attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 0],
							attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 1],
							attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 2]
						};
					
						vertex.color = color;

						if (uniqueVertices.count(vertex) == 0) {
							uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
							vertices.push_back(vertex);
						}

						indexes.push_back(uniqueVertices[vertex]);
						
						//indexes.push_back((uint32_t)indexes.size());
					}
					offset += shapes[i].mesh.num_face_vertices[j];
				}
			}
		}

		void CreateDescriptorSets(
			VkDevice device,
			VkDescriptorSetLayout descriptorSetLayoutForRigidBodyMesh,
			VkDescriptorPool descriptorPoolForRigidBodyMesh,
			std::vector<VkImageView> imageViews
		) {
			std::vector<VkDescriptorSetLayout> layouts(imageViews.size(), descriptorSetLayoutForRigidBodyMesh);

			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = descriptorPoolForRigidBodyMesh;
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

				vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
				writeDescriptorSets.resize(0);
			}
		}
		public:
		Transform transform;
		void recordDrawCalls(VkCommandBuffer commandBuffer, int imageIndex) {
			if (isCreated) {
				vkCmdBindPipeline(
					commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					graphicsPipelineForRigidBodyMesh.get()
				);
				VkBuffer buffers[] = { vertexBuffer.get() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineForRigidBodyMesh.getPipelineLayout(),
					0, 1, &descriptorSets[imageIndex], 0, nullptr);

				vkCmdBindIndexBuffer(commandBuffer, indexBuffer.get(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer, (uint32_t)indexes.size(), 1, 0, 0, 0);
			}
		}

		VkBuffer getVertexBuffer() {
			if (isCreated) {
				return vertexBuffer.get();
			}
			else {
				return VK_NULL_HANDLE;
			}
		}

		VkBuffer getIndexBuffer() {
			if (isCreated) {
				return indexBuffer.get();
			}
			else {
				return VK_NULL_HANDLE;
			}
		}

		VkDescriptorSet getDescriptorSetByIndex(int index) {
			if (isCreated) {

				return descriptorSets[index];
			}
			else {
				return VK_NULL_HANDLE;
			}
		}

		VkDescriptorSet* pGetDescriptorSetByIndex(int index) {
			if (isCreated) {
				return &descriptorSets[index];
			}
			else {
				return VK_NULL_HANDLE;
			}
		}

		std::vector<WireframeVertex> *getVertices() {
			if (isCreated) {
				return &vertices;
			}else{
				return nullptr;
			}
		}

		std::vector<uint32_t> getIndexes() {
			if (isCreated) {
				return indexes;
			}
		}
		
		void createMesh(std::string modelPath, glm::vec3 color) {
			loadModel(modelPath, color);

			vertexBuffer.CreateVertexBuffer(physicalDevice.get(), device.get(), device.getGraphicsQueue(), commandPool.get(),
				vertices.data(), sizeof(vertices[0]) * vertices.size());
			indexBuffer.CreateIndexBuffer(physicalDevice.get(), device.get(), device.getGraphicsQueue(), commandPool.get(),
				indexes.data(), sizeof(indexes[0]) * indexes.size());

			uniformBuffersMVP.resize(swapchain.pGetImageViews()->size());


			for (size_t i = 0; i < swapchain.pGetImageViews()->size(); i++) {
				uniformBuffersMVP[i].CreateUniformBuffer(physicalDevice.get(), device.get(), sizeof(UniformObjectMVP));
			}

			CreateDescriptorSets(device.get(), *setLayoutForRigidBodyMesh.pGet(), descriptorPoolForMesh.get(), *swapchain.pGetImageViews());
			isCreated = true;
		}
		void updateUniformBuffers(uint32_t imageIndex, VkDevice device, VkExtent2D extent) {
			if (isCreated) {
				//rotationMatrix = glm::toMat4(quaternion);
				ubo.model = transform.getMatrixProduct();
				ubo.view = camera.getView();
				ubo.proj = camera.getProjectionMatrix();

				ubo.proj[1][1] *= -1;
				void* data;
				vkMapMemory(device, uniformBuffersMVP[imageIndex].getSize(), 0, sizeof(ubo), 0, &data);
				memcpy(data, &ubo, sizeof(ubo));
				vkUnmapMemory(device, uniformBuffersMVP[imageIndex].getSize());
			}
		}
		void destroy() {
			if (isCreated) {
				vkFreeDescriptorSets(device.get(), descriptorPoolForMesh.get(), (uint32_t)descriptorSets.size(), descriptorSets.data());
				for (size_t i = 0; i < uniformBuffersMVP.size(); i++) {
					uniformBuffersMVP[i].destroy(device.get());
				}
				vertexBuffer.destroy(device.get());
				indexBuffer.destroy(device.get());
				isCreated = false;
			}
		}
		};

}
