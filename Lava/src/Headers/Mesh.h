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
	// Данные для меша
	private:
		std::string										MeshPath;
		std::vector<DataTypes::Vertex>					Vertices;
		std::vector<uint32_t>						    Indexes;
		std::unordered_map<DataTypes::Vertex, uint32_t> UniqueVertices{};

	// Буферные объекты (хранение вершин, индексов, изображений)
		Texture											mTexture;
		VulkanBuffers::VertexBuffer					    VertexBuffer;
		VulkanBuffers::IndexBuffer						IndexBuffer;

	// Объекты для задания дескрипторов 
		DataTypes::U_Struct_MVP							MVP{};
		DataTypes::U_Struct_Material					Material{};
		std::vector<VkDescriptorSet>				    DescriptorSets;

	// Буферы для передачи данных в Uniform'ы
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersMVP;
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersSpotLightAttributes;
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersDebugCameraPos;
		std::vector<VulkanBuffers::UniformBuffer>		UniformBuffersMaterial;
	
		void LoadModel(std::string modelPath) {
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::material_t> materials;
			std::vector<tinyobj::shape_t> shapes;
			std::string warn, err;

			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) throw std::runtime_error("FUCK");
			
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

						vertex.UVmap = {

						attrib.texcoords[2 * shapes[i].mesh.indices[k + offset].texcoord_index + 0],
						1.0f - attrib.texcoords[2 * shapes[i].mesh.indices[k + offset].texcoord_index + 1]

						};

						vertex.color = { 1.0f,1.0f,1.0f };

						vertex.normals = {

						attrib.normals[3 * shapes[i].mesh.indices[k + offset].normal_index + 0],
						attrib.normals[3 * shapes[i].mesh.indices[k + offset].normal_index + 1],
						attrib.normals[3 * shapes[i].mesh.indices[k + offset].normal_index + 2]

						};

						if (UniqueVertices.count(vertex)==0) {
							UniqueVertices[vertex] = static_cast<uint32_t>(Vertices.size());
							Vertices.push_back(vertex);
						}

						Indexes.push_back(UniqueVertices[vertex]);
						//indexes.push_back((uint32_t)indexes.size());
					}
					offset += shapes[i].mesh.num_face_vertices[j];
				}
			}
		}
		
		void CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForGameObjects,
			VkDescriptorPool descriptorPoolForGameObjects, std::vector<VkImageView> swapchainImageViews){

			std::vector<VkDescriptorSetLayout> layouts(swapchainImageViews.size(), descriptorSetLayoutForGameObjects);

			VkDescriptorSetAllocateInfo allocateInfo{};
			//
			{
				allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocateInfo.descriptorPool = descriptorPoolForGameObjects;
				allocateInfo.descriptorSetCount = (uint32_t)layouts.size();
				allocateInfo.pSetLayouts = layouts.data();
			}
			

			DescriptorSets.resize(layouts.size());

			VkResult result = vkAllocateDescriptorSets(device, &allocateInfo, DescriptorSets.data());
			if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate descriptor sets");
			

			std::vector<VkWriteDescriptorSet> writeDescriptorSets;
			for (size_t i = 0; i < DescriptorSets.size(); i++) {			
				VkDescriptorBufferInfo bufferInfo{};
				//
				{
					bufferInfo.buffer = UniformBuffersMVP[i].Get();
					bufferInfo.offset = 0;
					bufferInfo.range = sizeof(DataTypes::U_Struct_MVP);
				}
				
				VkWriteDescriptorSet mvpWriteDescriptorSet{};
				//
				{
					mvpWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					mvpWriteDescriptorSet.descriptorCount = 1;
					mvpWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					mvpWriteDescriptorSet.dstSet = DescriptorSets[i];
					mvpWriteDescriptorSet.dstBinding = 0;
					mvpWriteDescriptorSet.dstArrayElement = 0;
					mvpWriteDescriptorSet.pBufferInfo = &bufferInfo;
					writeDescriptorSets.push_back(mvpWriteDescriptorSet);
				}
				
				VkDescriptorImageInfo textureInfo{};
				//
				{
					textureInfo.imageView = mTexture.GetImageView();
					textureInfo.sampler = mTexture.GetImageSampler();
					textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
				
				VkWriteDescriptorSet textureWriteDescriptorSet{};
				//
				{
					textureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					textureWriteDescriptorSet.descriptorCount = 1;
					textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					textureWriteDescriptorSet.dstSet = DescriptorSets[i];
					textureWriteDescriptorSet.dstBinding = 1;
					textureWriteDescriptorSet.dstArrayElement = 0;
					textureWriteDescriptorSet.pImageInfo = &textureInfo;
					writeDescriptorSets.push_back(textureWriteDescriptorSet);
				}

				VkDescriptorBufferInfo bufferInfo2{};
				//
				{
					bufferInfo2.buffer = UniformBuffersSpotLightAttributes[i].Get();
					bufferInfo2.offset = 0;
					bufferInfo2.range = sizeof(DataTypes::U_Struct_SpotlightAttributes);
				}
				
				VkWriteDescriptorSet spotLightAttributesWriteDescriptorSet{};
				//
				{
					spotLightAttributesWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					spotLightAttributesWriteDescriptorSet.descriptorCount = 1;
					spotLightAttributesWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					spotLightAttributesWriteDescriptorSet.dstSet = DescriptorSets[i];
					spotLightAttributesWriteDescriptorSet.dstBinding = 2;
					spotLightAttributesWriteDescriptorSet.dstArrayElement = 0;
					spotLightAttributesWriteDescriptorSet.pBufferInfo = &bufferInfo2;
					writeDescriptorSets.push_back(spotLightAttributesWriteDescriptorSet);
				}
				
				VkDescriptorBufferInfo bufferInfo3{};
				//
				{
					bufferInfo3.buffer = UniformBuffersDebugCameraPos[i].Get();
					bufferInfo3.offset = 0;
					bufferInfo3.range = sizeof(DataTypes::U_Struct_DebugCameraPos);
				}
				
				VkWriteDescriptorSet debugCameraPosWriteDescriptorSet{};
				//
				{
					debugCameraPosWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					debugCameraPosWriteDescriptorSet.descriptorCount = 1;
					debugCameraPosWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					debugCameraPosWriteDescriptorSet.dstSet = DescriptorSets[i];
					debugCameraPosWriteDescriptorSet.dstBinding = 3;
					debugCameraPosWriteDescriptorSet.dstArrayElement = 0;
					debugCameraPosWriteDescriptorSet.pBufferInfo = &bufferInfo3;
					writeDescriptorSets.push_back(debugCameraPosWriteDescriptorSet);
				}
				
				VkDescriptorBufferInfo bufferInfo4{};
				//
				{
					bufferInfo4.buffer = UniformBuffersMaterial[i].Get();
					bufferInfo4.offset = 0;
					bufferInfo4.range = sizeof(DataTypes::U_Struct_Material);
				}
				
				VkWriteDescriptorSet materialWriteDescriptorSet{};
				//
				{
					materialWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					materialWriteDescriptorSet.descriptorCount = 1;
					materialWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					materialWriteDescriptorSet.dstSet = DescriptorSets[i];
					materialWriteDescriptorSet.dstBinding = 4;
					materialWriteDescriptorSet.dstArrayElement = 0;
					materialWriteDescriptorSet.pBufferInfo = &bufferInfo4;
					writeDescriptorSets.push_back(materialWriteDescriptorSet);
				}
							
				vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
				writeDescriptorSets.resize(0);
			}
		}
	
		public:

		VkBuffer GetVertexBuffer() {
			if (IsCreated) return VertexBuffer.Get();
			else return VK_NULL_HANDLE;
		}

		VkBuffer GetIndexBuffer() {
			if (IsCreated) return IndexBuffer.Get();
			else return VK_NULL_HANDLE;
		}

		std::string GetMeshPath() {
			return MeshPath;
		}

		std::vector<DataTypes::Vertex>* GetVertices() {
			if (IsCreated) return &Vertices;
			else return nullptr;
		}

		std::vector<uint32_t> GetIndexes() {
			if (IsCreated) return Indexes;
		}

		DataTypes::U_Struct_Material GetMaterial() {
			if (IsCreated) return Material;

		}

		void SetBaseColorTexture(std::string path) {
			mTexture.DestroyTexture(gDevice.Get());
			mTexture.CreateTexture(
				gPhysicalDevice.Get(), gDevice.Get(), gDevice.GetGraphicsQueue(), gCommandPool.Get(), path
			);
			CreateDescriptorSets(gDevice.Get(), *gSetLayoutForMesh.PGet(), gDescriptorPoolForMesh.Get(), *gSwapchain.PGetImageViews());
		}

		void SetMaterial(DataTypes::U_Struct_Material mat) {
			if (IsCreated) {
				Material = mat;
			}
		}

		bool IsCreated = false;
		
		Transform vTransform;
		
		void Draw(VkCommandBuffer commandBuffer, int imageIndex) {
			if (IsCreated){
				vkCmdBindPipeline(
					commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					gGraphicsPipelineForMesh.Get()
				);

				if (ENABLE_DYNAMIC_VIEWPORT){
					vkCmdSetViewport(commandBuffer, 0, 1, &gEditor3DView);
					vkCmdSetScissor(commandBuffer, 0, 1, &gEditor3DScissors);
				}

				VkBuffer buffers[] = { VertexBuffer.Get() };
				VkDeviceSize offsets[] = { 0 };

			
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gGraphicsPipelineForMesh.GetPipelineLayout(),
					0, 1, &DescriptorSets[imageIndex], 0, nullptr);

				vkCmdBindIndexBuffer(commandBuffer, IndexBuffer.Get(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffer, (uint32_t)Indexes.size(), 1, 0, 0, 0);
			}
		}

		void CreateMesh(std::string modelPath){
			Material = { 0.1f,1.0f,5.0f };
			MeshPath = modelPath;

			LoadModel(modelPath);

			//Создание пустой текстуры (прозрачный один пиксель)
			//Это необходимо, чтобы uniform'а в шейдере не была пустой
			mTexture.CreateTexture(gPhysicalDevice.Get(), 
								   gDevice.Get(), 
								   gDevice.GetGraphicsQueue(), 
								   gCommandPool.Get(), "");

			//Создание буферов
			{
				VertexBuffer.CreateVertexBuffer(gPhysicalDevice.Get(),
					gDevice.Get(),
					gDevice.GetGraphicsQueue(),
					gCommandPool.Get(),
					Vertices.data(),
					sizeof(Vertices[0]) * Vertices.size());

				IndexBuffer.CreateIndexBuffer(gPhysicalDevice.Get(),
					gDevice.Get(),
					gDevice.GetGraphicsQueue(),
					gCommandPool.Get(),
					Indexes.data(), sizeof(Indexes[0]) * Indexes.size());

				UniformBuffersMVP.resize(gSwapchain.PGetImageViews()->size());
				UniformBuffersSpotLightAttributes.resize(gSwapchain.PGetImageViews()->size()*MAX_SPOTLIGHTS);
				UniformBuffersDebugCameraPos.resize(gSwapchain.PGetImageViews()->size());
				UniformBuffersMaterial.resize(gSwapchain.PGetImageViews()->size());

				for (size_t i = 0; i < gSwapchain.PGetImageViews()->size(); i++) {
					UniformBuffersMVP[i].CreateUniformBuffer(gPhysicalDevice.Get(), gDevice.Get(), sizeof(DataTypes::U_Struct_MVP));
					UniformBuffersDebugCameraPos[i].CreateUniformBuffer(gPhysicalDevice.Get(), gDevice.Get(), sizeof(DataTypes::U_Struct_DebugCameraPos));
					UniformBuffersMaterial[i].CreateUniformBuffer(gPhysicalDevice.Get(), gDevice.Get(), sizeof(DataTypes::U_Struct_Material));
				}

				for (size_t i = 0; i < UniformBuffersSpotLightAttributes.size(); i++) {
					UniformBuffersSpotLightAttributes[i].CreateUniformBuffer(gPhysicalDevice.Get(), gDevice.Get(), sizeof(DataTypes::U_Struct_SpotlightAttributes));
				}

			}

			CreateDescriptorSets(
				gDevice.Get(), *gSetLayoutForMesh.PGet(), gDescriptorPoolForMesh.Get(), *gSwapchain.PGetImageViews()
			);

			IsCreated = true;
		}

		void UpdateUniforms(uint32_t imageIndex, VkDevice device) {
			if (IsCreated){
				MVP.model = vTransform.gMatrixProduct();
				MVP.view = debugCamera.GetView();
				MVP.proj = debugCamera.GetProjectionMatrix();

				MVP.proj[1][1] *= -1;

				void* data;
				vkMapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory(), 0, sizeof(MVP), 0, &data);
				memcpy(data, &MVP, sizeof(MVP));
				vkUnmapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory());

				DataTypes::U_Struct_SpotlightAttributes light;
				light.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
				light.lightPosition = glm::vec3(0.0f, 15.0f, 12.0f);

				vkMapMemory(device, UniformBuffersSpotLightAttributes[imageIndex].GetDeviceMemory(), 0, sizeof(DataTypes::U_Struct_SpotlightAttributes), 0, &data);
				memcpy(data, &light, sizeof(DataTypes::U_Struct_SpotlightAttributes));
				vkUnmapMemory(device, UniformBuffersSpotLightAttributes[imageIndex].GetDeviceMemory());

				DataTypes::U_Struct_DebugCameraPos debugCameraPos;
				debugCameraPos.pos = debugCamera.GetPosition();

				vkMapMemory(device, UniformBuffersDebugCameraPos[imageIndex].GetDeviceMemory(), 0, sizeof(DataTypes::U_Struct_DebugCameraPos), 0, &data);
				memcpy(data, &debugCameraPos, sizeof(DataTypes::U_Struct_DebugCameraPos));
				vkUnmapMemory(device, UniformBuffersDebugCameraPos[imageIndex].GetDeviceMemory());

				vkMapMemory(device, UniformBuffersMaterial[imageIndex].GetDeviceMemory(), 0, sizeof(DataTypes::U_Struct_Material), 0, &data);
				memcpy(data, &Material, sizeof(DataTypes::U_Struct_Material));
				vkUnmapMemory(device, UniformBuffersMaterial[imageIndex].GetDeviceMemory());
			}
		}

		void Destroy() {
			if (IsCreated) {
				mTexture.DestroyTexture(gDevice.Get());
				vkFreeDescriptorSets(gDevice.Get(), gDescriptorPoolForMesh.Get(), (uint32_t)DescriptorSets.size(), DescriptorSets.data());
				for (size_t i = 0; i < UniformBuffersMVP.size(); i++) {
					UniformBuffersMVP[i].Destroy(gDevice.Get());
					UniformBuffersSpotLightAttributes[i].Destroy(gDevice.Get());
					UniformBuffersDebugCameraPos[i].Destroy(gDevice.Get());
					UniformBuffersMaterial[i].Destroy(gDevice.Get());

				}
				VertexBuffer.Destroy(gDevice.Get());
				IndexBuffer.Destroy(gDevice.Get());
				IsCreated = false;
			}
		}

		
	};



	class WireframeMesh {
		private:
		std::vector<DataTypes::WireframeVertex> Vertices;
		std::vector<uint32_t>					Indexes;
		std::unordered_map<DataTypes::WireframeVertex, uint32_t> UniqueVertices{};

		VulkanBuffers::VertexBuffer				VertexBuffer;
		VulkanBuffers::IndexBuffer				IndexBuffer;

		DataTypes::U_Struct_MVP MVP{};

		std::vector<VkDescriptorSet>			DescriptorSets;

		std::vector<VulkanBuffers::UniformBuffer> UniformBuffersMVP;

		void LoadModel(std::string modelPath, glm::vec3 color) {
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::material_t> materials;
			std::vector<tinyobj::shape_t> shapes;
			std::string warn, err;
			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) throw std::runtime_error(warn + err);
			
			for (size_t i = 0; i < shapes.size(); i++) {
				size_t offset = 0;
				for (size_t j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++) {
					for (size_t k = 0; k < shapes[i].mesh.num_face_vertices[j]; k++) {

						DataTypes::WireframeVertex vertex{};
						vertex.pos = {
							attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 0],
							attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 1],
							attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 2]
						};
					
						vertex.color = color;

						if (UniqueVertices.count(vertex) == 0) {
							UniqueVertices[vertex] = static_cast<uint32_t>(Vertices.size());
							Vertices.push_back(vertex);
						}

						Indexes.push_back(UniqueVertices[vertex]);
						
						//indexes.push_back((uint32_t)indexes.size());
					}
					offset += shapes[i].mesh.num_face_vertices[j];
				}
			}
		}

		void CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForRigidBodyMesh,
			VkDescriptorPool descriptorPoolForRigidBodyMesh, std::vector<VkImageView> imageViews){

			std::vector<VkDescriptorSetLayout> layouts(imageViews.size(), descriptorSetLayoutForRigidBodyMesh);

			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = descriptorPoolForRigidBodyMesh;
			allocateInfo.descriptorSetCount = (uint32_t)layouts.size();
			allocateInfo.pSetLayouts = layouts.data();

			DescriptorSets.resize(layouts.size());

			VkResult result = vkAllocateDescriptorSets(device, &allocateInfo, DescriptorSets.data());
			if (result != VK_SUCCESS){
				std::cout << result << std::endl;
				throw std::runtime_error("Failed to allocate descriptor sets");
			}

			std::vector<VkWriteDescriptorSet> writeDescriptorSets;
			for (size_t i = 0; i < DescriptorSets.size(); i++) {
				VkDescriptorBufferInfo bufferInfo{};
				VkWriteDescriptorSet mvpWriteDescriptorSet{};
				{
					bufferInfo.buffer = UniformBuffersMVP[i].Get();
					bufferInfo.offset = 0;
					bufferInfo.range = sizeof(DataTypes::U_Struct_MVP);


					mvpWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					mvpWriteDescriptorSet.descriptorCount = 1;
					mvpWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					mvpWriteDescriptorSet.dstSet = DescriptorSets[i];
					mvpWriteDescriptorSet.dstBinding = 0;
					mvpWriteDescriptorSet.dstArrayElement = 0;
					mvpWriteDescriptorSet.pBufferInfo = &bufferInfo;
					writeDescriptorSets.push_back(mvpWriteDescriptorSet);
				
				}
				

				vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
				writeDescriptorSets.resize(0);
			}
		}
	public:
		bool IsCreated = false;
		Transform Transform;

		void Draw(VkCommandBuffer commandBuffer, int imageIndex)	{
			if (IsCreated)	{
				vkCmdBindPipeline(
					commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					gGraphicsPipelineForRigidBodyMesh.Get()
				);

				VkBuffer buffers[] = { VertexBuffer.Get() };

				VkDeviceSize offsets[] = { 0 };

				if (ENABLE_DYNAMIC_VIEWPORT)	{
					vkCmdSetViewport(commandBuffer, 0, 1, &gEditor3DView);
					vkCmdSetScissor(commandBuffer, 0, 1, &gEditor3DScissors);
				}

				vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gGraphicsPipelineForRigidBodyMesh.GetPipelineLayout(),
					0, 1, &DescriptorSets[imageIndex], 0, nullptr);

				vkCmdBindIndexBuffer(commandBuffer, IndexBuffer.Get(), 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(commandBuffer, (uint32_t)Indexes.size(), 1, 0, 0, 0);
			}
		}

		VkBuffer GetVertexBuffer()	{
			if (IsCreated) return VertexBuffer.Get();
			else return VK_NULL_HANDLE;
		}

		VkBuffer GetIndexBuffer(){
			if (IsCreated) return IndexBuffer.Get();
			else VK_NULL_HANDLE;
		}

		std::vector<DataTypes::WireframeVertex> *GetVertices(){
			if (IsCreated) return &Vertices;
			else return nullptr;
		}

		std::vector<uint32_t> GetIndexes(){
			if (IsCreated) return Indexes;
		}
		
		void CreateMesh(std::string modelPath, glm::vec3 color){
			LoadModel(modelPath, color);

			VertexBuffer.CreateVertexBuffer(gPhysicalDevice.Get(), gDevice.Get(), gDevice.GetGraphicsQueue(), gCommandPool.Get(),
				Vertices.data(), sizeof(Vertices[0]) * Vertices.size());
			IndexBuffer.CreateIndexBuffer(gPhysicalDevice.Get(), gDevice.Get(), gDevice.GetGraphicsQueue(), gCommandPool.Get(),
				Indexes.data(), sizeof(Indexes[0]) * Indexes.size());

			UniformBuffersMVP.resize(gSwapchain.PGetImageViews()->size());

			for (size_t i = 0; i < gSwapchain.PGetImageViews()->size(); i++) {
				UniformBuffersMVP[i].CreateUniformBuffer(gPhysicalDevice.Get(), gDevice.Get(), sizeof(DataTypes::U_Struct_MVP));
			}

			CreateDescriptorSets(gDevice.Get(), *gSetLayoutForRigidBodyMesh.PGet(), gDescriptorPoolForMesh.Get(), *gSwapchain.PGetImageViews());
			IsCreated = true;
		}

		void UpdateUniforms(uint32_t imageIndex, VkDevice device)	{
			if (IsCreated)	{
				//rotationMatrix = glm::toMat4(quaternion);
				MVP.model = Transform.gMatrixProduct();
				MVP.view = debugCamera.GetView();
				MVP.proj = debugCamera.GetProjectionMatrix();

				MVP.proj[1][1] *= -1;
				void* data;
				vkMapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory(), 0, sizeof(MVP), 0, &data);
				memcpy(data, &MVP, sizeof(MVP));
				vkUnmapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory());
			}
		}

		void Destroy() {
			if (IsCreated) {
				vkFreeDescriptorSets(gDevice.Get(), gDescriptorPoolForMesh.Get(), (uint32_t)DescriptorSets.size(), DescriptorSets.data());
				for (size_t i = 0; i < UniformBuffersMVP.size(); i++) {
					UniformBuffersMVP[i].Destroy(gDevice.Get());
				}
				VertexBuffer.Destroy(gDevice.Get());
				IndexBuffer.Destroy(gDevice.Get());
				IsCreated = false;
			}
		}
	
	};

}
