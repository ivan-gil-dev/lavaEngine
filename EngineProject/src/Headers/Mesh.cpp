#include "Mesh.h"
#define TINYOBJLOADER_STATIC
#include    "../../vendor/tiny_obj_loader.h"

void Engine::Mesh::LoadModel(std::string modelPath) {

	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./"; // Path to material files

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(modelPath, reader_config)) {
		if (!reader.Error().empty()) {

			std::string error = "TinyObjReader: " ;
			error.append(reader.Error());

			throw std::runtime_error(error);
		}
		exit(1);
	}

	/*if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}*/

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				DataTypes::MeshVertex_t vertex;
				
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				vertex.pos = {
					vx,
					vy,
					vz
				};

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (idx.normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

					vertex.normals = {
						nx,
						ny,
						nz
					};
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (idx.texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					vertex.UVmap = {
						tx,
						1.0f - ty
					};
				}


				 tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
				 tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
				 tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

				vertex.color = { red,green,blue };

				if (UniqueVertices.count(vertex) == 0) {
					UniqueVertices[vertex] = static_cast<uint32_t>(Vertices.size());
					Vertices.push_back(vertex);
				}

				Indexes.push_back(UniqueVertices[vertex]);
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}
}

void Engine::Mesh::CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForGameObjects, 
	VkDescriptorPool descriptorPoolForGameObjects, std::vector<VkImageView> swapchainImageViews) {
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
			bufferInfo.range = sizeof(DataTypes::MVP_t);
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
			bufferInfo2.range = sizeof(DataTypes::SpotlightAttributes_t) * MAX_SPOTLIGHTS;
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
			bufferInfo3.range = sizeof(DataTypes::CameraPos_t);
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
			bufferInfo4.range = sizeof(DataTypes::Material_t);
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

VkBuffer Engine::Mesh::GetVertexBuffer() {
	return VertexBuffer.Get();
}

VkBuffer Engine::Mesh::GetIndexBuffer() {
	return IndexBuffer.Get();
}

std::string Engine::Mesh::pGetMeshPath() {
	return MeshPath;
}

std::vector<Engine::DataTypes::MeshVertex_t>* Engine::Mesh::GetVertices() {
	return &Vertices;
}

std::vector<glm::uint32_t> Engine::Mesh::GetIndexes() {
	return Indexes;
}

Engine::DataTypes::Material_t Engine::Mesh::GetMaterial() {
	return Material;
}

void Engine::Mesh::SetBaseColorTexture(std::string path) {
	mTexture.DestroyTexture(Globals::gDevice.Get());
	vkFreeDescriptorSets(Globals::gDevice.Get(), Globals::gDescriptorPoolForMesh.Get(),
		(uint32_t)DescriptorSets.size(), DescriptorSets.data());
	mTexture.CreateTexture(
		Globals::gPhysicalDevice.Get(), Globals::gDevice.Get(), Globals::gDevice.GetGraphicsQueue(), Globals::gCommandPool.Get(), path
	);
	CreateDescriptorSets(Globals::gDevice.Get(), Globals::gSetLayoutForMesh.Get(),
		Globals::gDescriptorPoolForMesh.Get(), *Globals::gSwapchain.PGetImageViews());
}

void Engine::Mesh::SetMaterial(DataTypes::Material_t mat) {
	Material = mat;
}

void Engine::Mesh::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		Globals::gGraphicsPipelineForMesh.Get()
	);

	if (ENABLE_DYNAMIC_VIEWPORT) {
		vkCmdSetViewport(commandBuffer, 0, 1, &Globals::gEditor3DView);
		vkCmdSetScissor(commandBuffer, 0, 1, &Globals::gEditor3DScissors);
	}

	VkBuffer buffers[] = { VertexBuffer.Get() };
	VkDeviceSize offsets[] = { 0 };


	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Globals::gGraphicsPipelineForMesh.GetPipelineLayout(),
		0, 1, &DescriptorSets[imageIndex], 0, nullptr);

	vkCmdBindIndexBuffer(commandBuffer, IndexBuffer.Get(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, (uint32_t)Indexes.size(), 1, 0, 0, 0);
}

void Engine::Mesh::CreateMesh(std::string modelPath) {
	Material = { 0.1f,1.0f,2.0f };
	MeshPath = modelPath;

	LoadModel(modelPath);

	//<1x1 текстура (0,0,0,255)> 
	mTexture.CreateTexture(Globals::gPhysicalDevice.Get(),
		Globals::gDevice.Get(),
		Globals::gDevice.GetGraphicsQueue(),
		Globals::gCommandPool.Get(), "");

	//Создание буферов
	{
		VertexBuffer.CreateVertexBuffer(Globals::gPhysicalDevice.Get(),
			Globals::gDevice.Get(),
			Globals::gDevice.GetGraphicsQueue(),
			Globals::gCommandPool.Get(),
			Vertices.data(),
			sizeof(Vertices[0]) * Vertices.size());

		IndexBuffer.CreateIndexBuffer(Globals::gPhysicalDevice.Get(),
			Globals::gDevice.Get(),
			Globals::gDevice.GetGraphicsQueue(),
			Globals::gCommandPool.Get(),
			Indexes.data(), sizeof(Indexes[0]) * Indexes.size());

		UniformBuffersMVP.resize(Globals::gSwapchain.PGetImageViews()->size());
		UniformBuffersSpotLightAttributes.resize(Globals::gSwapchain.PGetImageViews()->size());
		UniformBuffersDebugCameraPos.resize(Globals::gSwapchain.PGetImageViews()->size());
		UniformBuffersMaterial.resize(Globals::gSwapchain.PGetImageViews()->size());

		for (size_t i = 0; i < Globals::gSwapchain.PGetImageViews()->size(); i++) {
			UniformBuffersMVP[i].CreateUniformBuffer(Globals::gPhysicalDevice.Get(), Globals::gDevice.Get(), sizeof(DataTypes::MVP_t));
			UniformBuffersDebugCameraPos[i].CreateUniformBuffer(Globals::gPhysicalDevice.Get(), Globals::gDevice.Get(), sizeof(DataTypes::CameraPos_t));
			UniformBuffersMaterial[i].CreateUniformBuffer(Globals::gPhysicalDevice.Get(), Globals::gDevice.Get(), sizeof(DataTypes::Material_t));
			UniformBuffersSpotLightAttributes[i].CreateUniformBuffer(Globals::gPhysicalDevice.Get(), Globals::gDevice.Get(),
				sizeof(DataTypes::SpotlightAttributes_t) * MAX_SPOTLIGHTS);
		}
	}

	CreateDescriptorSets(
		Globals::gDevice.Get(), Globals::gSetLayoutForMesh.Get(),
		Globals::gDescriptorPoolForMesh.Get(), *Globals::gSwapchain.PGetImageViews()
	);
}

void Engine::Mesh::UpdateUniforms(uint32_t imageIndex, VkDevice device, 
	glm::mat4 TransformMatrixProduct, std::vector<DataTypes::SpotlightAttributes_t*> spotlightAttributes) {

	MVP.model = TransformMatrixProduct;
	MVP.view = Globals::debugCamera.GetView();
	MVP.proj = Globals::debugCamera.GetProjectionMatrix();

	MVP.proj[1][1] *= -1;

	void* data;
	vkMapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory(), 0, sizeof(MVP), 0, &data);
	memcpy(data, &MVP, sizeof(MVP));
	vkUnmapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory());


	vkMapMemory(device, UniformBuffersMaterial[imageIndex].GetDeviceMemory(), 0, sizeof(DataTypes::Material_t), 0, &data);
	memcpy(data, &Material, sizeof(DataTypes::Material_t));
	vkUnmapMemory(device, UniformBuffersMaterial[imageIndex].GetDeviceMemory());

	std::vector<DataTypes::SpotlightAttributes_t> buffer_SpotlightAttributes(MAX_SPOTLIGHTS);

	for (size_t i = 0; i < buffer_SpotlightAttributes.size(); i++) {
		if (i < spotlightAttributes.size()){
			buffer_SpotlightAttributes[i] = *spotlightAttributes[i];
		}

		else {
			buffer_SpotlightAttributes[i].lightColor = glm::vec3(0, 0, 0);
			buffer_SpotlightAttributes[i].lightPosition = glm::vec3(0, 0, 0);
		}
	}

	vkMapMemory(device, UniformBuffersSpotLightAttributes[imageIndex].GetDeviceMemory(), 0,
		sizeof(DataTypes::SpotlightAttributes_t) * buffer_SpotlightAttributes.size(), 0, &data);
	memcpy(data, buffer_SpotlightAttributes.data(), sizeof(DataTypes::SpotlightAttributes_t) * buffer_SpotlightAttributes.size());
	vkUnmapMemory(device, UniformBuffersSpotLightAttributes[imageIndex].GetDeviceMemory());

	DataTypes::CameraPos_t debugCameraPos;
	debugCameraPos.pos = Globals::debugCamera.GetPosition();

	vkMapMemory(device, UniformBuffersDebugCameraPos[imageIndex].GetDeviceMemory(), 0, sizeof(DataTypes::CameraPos_t), 0, &data);
	memcpy(data, &debugCameraPos, sizeof(DataTypes::CameraPos_t));
	vkUnmapMemory(device, UniformBuffersDebugCameraPos[imageIndex].GetDeviceMemory());
}

void Engine::Mesh::Destroy() {
	mTexture.DestroyTexture(Globals::gDevice.Get());
	vkFreeDescriptorSets(Globals::gDevice.Get(), Globals::gDescriptorPoolForMesh.Get(),
		(uint32_t)DescriptorSets.size(), DescriptorSets.data());
	for (size_t i = 0; i < UniformBuffersMVP.size(); i++) {
		UniformBuffersMVP[i].Destroy(Globals::gDevice.Get());
		UniformBuffersDebugCameraPos[i].Destroy(Globals::gDevice.Get());
		UniformBuffersMaterial[i].Destroy(Globals::gDevice.Get());
		UniformBuffersSpotLightAttributes[i].Destroy(Globals::gDevice.Get());

	}
	VertexBuffer.Destroy(Globals::gDevice.Get());
	IndexBuffer.Destroy(Globals::gDevice.Get());
}

void Engine::WireframeMesh::LoadModel(std::string modelPath, glm::vec3 color) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::material_t> materials;
	std::vector<tinyobj::shape_t> shapes;
	std::string warn, err;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) throw std::runtime_error(warn + err);

	for (size_t i = 0; i < shapes.size(); i++) {
		size_t offset = 0;
		for (size_t j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++) {
			for (size_t k = 0; k < shapes[i].mesh.num_face_vertices[j]; k++) {

				DataTypes::WireframeMeshVertex_t vertex{};
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

void Engine::WireframeMesh::CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForRigidBodyMesh, 
	VkDescriptorPool descriptorPoolForRigidBodyMesh, std::vector<VkImageView> imageViews) {
	std::vector<VkDescriptorSetLayout> layouts(imageViews.size(), descriptorSetLayoutForRigidBodyMesh);

	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = descriptorPoolForRigidBodyMesh;
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
		VkWriteDescriptorSet mvpWriteDescriptorSet{};
		{
			bufferInfo.buffer = UniformBuffersMVP[i].Get();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(DataTypes::MVP_t);


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

Engine::WireframeMesh::WireframeMesh() {

}

void Engine::WireframeMesh::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		Globals::gGraphicsPipelineForRigidBodyMesh.Get()
	);

	VkBuffer buffers[] = { VertexBuffer.Get() };

	VkDeviceSize offsets[] = { 0 };

	if (ENABLE_DYNAMIC_VIEWPORT) {
		vkCmdSetViewport(commandBuffer, 0, 1, &Globals::gEditor3DView);
		vkCmdSetScissor(commandBuffer, 0, 1, &Globals::gEditor3DScissors);
	}

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Globals::gGraphicsPipelineForRigidBodyMesh.GetPipelineLayout(),
		0, 1, &DescriptorSets[imageIndex], 0, nullptr);

	vkCmdBindIndexBuffer(commandBuffer, IndexBuffer.Get(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, (uint32_t)Indexes.size(), 1, 0, 0, 0);
}

VkBuffer Engine::WireframeMesh::GetVertexBuffer() {
	return VertexBuffer.Get();
}

VkBuffer Engine::WireframeMesh::GetIndexBuffer() {
	return IndexBuffer.Get();
}

std::vector<Engine::DataTypes::WireframeMeshVertex_t> Engine::WireframeMesh::GetVertices() {
	return Vertices;
}

std::vector<glm::uint32_t> Engine::WireframeMesh::GetIndexes() {
	return Indexes;
}

void Engine::WireframeMesh::CreateMesh(std::string modelPath, glm::vec3 color) {
	LoadModel(modelPath, color);

	VertexBuffer.CreateVertexBuffer(Globals::gPhysicalDevice.Get(), Globals::gDevice.Get(),
		Globals::gDevice.GetGraphicsQueue(), Globals::gCommandPool.Get(),
		Vertices.data(), sizeof(Vertices[0]) * Vertices.size());

	IndexBuffer.CreateIndexBuffer(Globals::gPhysicalDevice.Get(), Globals::gDevice.Get(),
		Globals::gDevice.GetGraphicsQueue(), Globals::gCommandPool.Get(),
		Indexes.data(), sizeof(Indexes[0]) * Indexes.size());

	UniformBuffersMVP.resize(Globals::gSwapchain.PGetImageViews()->size());

	for (size_t i = 0; i < Globals::gSwapchain.PGetImageViews()->size(); i++) {
		UniformBuffersMVP[i].CreateUniformBuffer(Globals::gPhysicalDevice.Get(), Globals::gDevice.Get(), sizeof(DataTypes::MVP_t));
	}

	CreateDescriptorSets(Globals::gDevice.Get(), Globals::gSetLayoutForRigidBodyMesh.Get(),
		Globals::gDescriptorPoolForMesh.Get(), *Globals::gSwapchain.PGetImageViews());
}

void Engine::WireframeMesh::UpdateUniforms(uint32_t imageIndex, VkDevice device) {

	MVP.model = Transform.GetMatrixProduct();
	MVP.view = Globals::debugCamera.GetView();
	MVP.proj = Globals::debugCamera.GetProjectionMatrix();

	MVP.proj[1][1] *= -1;
	void* data;
	vkMapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory(), 0, sizeof(MVP), 0, &data);
	memcpy(data, &MVP, sizeof(MVP));
	vkUnmapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory());
}

void Engine::WireframeMesh::Destroy() {
	vkFreeDescriptorSets(Globals::gDevice.Get(), Globals::gDescriptorPoolForMesh.Get(), 
		(uint32_t)DescriptorSets.size(), DescriptorSets.data());
	for (size_t i = 0; i < UniformBuffersMVP.size(); i++) {
		UniformBuffersMVP[i].Destroy(Globals::gDevice.Get());
	}
	VertexBuffer.Destroy(Globals::gDevice.Get());
	IndexBuffer.Destroy(Globals::gDevice.Get());
}

void Engine::CubemapMesh::CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForCubemap, 
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
		bufferInfo.buffer = UniformBuffersVP[i].Get();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(DataTypes::CubemapVP_t);

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
		textureInfo.imageView = mCubemapTexture.GetImageView();
		textureInfo.sampler = mCubemapTexture.GetImageSampler();
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

void Engine::CubemapMesh::LoadModel(std::string modelPath) {
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

				DataTypes::MeshVertex_t vertex{};
				vertex.pos = glm::vec3(
					attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 0],
					attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 1],
					attrib.vertices[3 * shapes[i].mesh.indices[k + offset].vertex_index + 2]
				);

				vertex.UVmap = glm::vec2(
					attrib.texcoords[2 * shapes[i].mesh.indices[k + offset].texcoord_index + 0],
					1.0f - attrib.texcoords[2 * shapes[i].mesh.indices[k + offset].texcoord_index + 1]

				);

				vertex.color = glm::vec3(
					attrib.colors[3 * shapes[i].mesh.indices[k + offset].vertex_index + 0],
					attrib.colors[3 * shapes[i].mesh.indices[k + offset].vertex_index + 1],
					attrib.colors[3 * shapes[i].mesh.indices[k + offset].vertex_index + 2]
				);

				vertex.normals = glm::vec3(
					attrib.normals[3 * shapes[i].mesh.indices[k + offset].normal_index + 0],
					attrib.normals[3 * shapes[i].mesh.indices[k + offset].normal_index + 1],
					attrib.normals[3 * shapes[i].mesh.indices[k + offset].normal_index + 2]
				);

				//std::cout << "Before " << vertex->pos.x << std::endl;

				Vertices.push_back(vertex);

				//td::cout << "After " << Vertices->at(Vertices->size() - 1).pos.x << std::endl;

				Indexes.push_back((uint32_t)Indexes.size());


			}
			offset += shapes[i].mesh.num_face_vertices[j];
		}
	}
}

Engine::CubemapMesh::CubemapMesh() {

}

void Engine::CubemapMesh::CreateCubemapMesh(std::vector<std::string> paths) {
	LoadModel("CoreAssets/cube.obj");

	for (int i = 0; i < Vertices.size(); i++) {
		CubeMapVertices.push_back(Vertices[i].pos);

	}

	mCubemapTexture.CreateCubemapTexture(
		Globals::gDevice.Get(),
		Globals::gPhysicalDevice.Get(),
		Globals::gCommandPool.Get(),
		Globals::gDevice.GetGraphicsQueue(),
		paths
	);

	UniformBuffersVP.resize(Globals::gSwapchain.PGetImageViews()->size());

	for (size_t i = 0; i < Globals::gSwapchain.PGetImageViews()->size(); i++) {
		UniformBuffersVP[i].CreateUniformBuffer(
			Globals::gPhysicalDevice.Get(),
			Globals::gDevice.Get(),
			sizeof(DataTypes::CubemapVP_t)
		);
	}

	CreateDescriptorSets(
		Globals::gDevice.Get(),
		Globals::gSetLayoutForCubemapObjects.Get(),
		Globals::gDescriptorPoolForCubemapObjects.Get(),
		*Globals::gSwapchain.PGetImageViews()
	);

	mVertexBuffer.CreateVertexBuffer(
		Globals::gPhysicalDevice.Get(),
		Globals::gDevice.Get(),
		Globals::gDevice.GetGraphicsQueue(),
		Globals::gCommandPool.Get(),
		CubeMapVertices.data(),
		sizeof(CubeMapVertices[0]) * CubeMapVertices.size());

	mIndexBuffer.CreateIndexBuffer(
		Globals::gPhysicalDevice.Get(),
		Globals::gDevice.Get(),
		Globals::gDevice.GetGraphicsQueue(),
		Globals::gCommandPool.Get(),
		Indexes.data(),
		sizeof(Indexes[0]) * Indexes.size());
}

void Engine::CubemapMesh::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		Globals::gGraphicsPipelineForCubemapObjects.Get()
	);

	if (ENABLE_DYNAMIC_VIEWPORT) {
		vkCmdSetViewport(commandBuffer, 0, 1, &Globals::gEditor3DView);
		vkCmdSetScissor(commandBuffer, 0, 1, &Globals::gEditor3DScissors);
	}

	VkBuffer buffers[] = { mVertexBuffer.Get() };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Globals::gGraphicsPipelineForCubemapObjects.GetPipelineLayout(),
		0, 1, &DescriptorSets[imageIndex], 0, nullptr);
	vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer.Get(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, (uint32_t)Indexes.size(), 1, 0, 0, 0);
}

void Engine::CubemapMesh::UpdateUniforms(uint32_t imageIndex, VkDevice device) {
	DataTypes::CubemapVP_t cubemapMp;
	cubemapMp.view = Globals::debugCamera.GetCubemapViewForVulkan();
	cubemapMp.projection = Globals::debugCamera.GetProjectionMatrix();
	cubemapMp.view[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	void* data;
	vkMapMemory(device, UniformBuffersVP[imageIndex].GetDeviceMemory(), 0, sizeof(cubemapMp), 0, &data);
	memcpy(data, &cubemapMp, sizeof(cubemapMp));
	vkUnmapMemory(device, UniformBuffersVP[imageIndex].GetDeviceMemory());
}

void Engine::CubemapMesh::Destroy() {
	mCubemapTexture.DestroyTexture(Globals::gDevice.Get());
	vkFreeDescriptorSets(Globals::gDevice.Get(),
		Globals::gDescriptorPoolForCubemapObjects.Get(),
		(uint32_t)DescriptorSets.size(),
		DescriptorSets.data());

	for (size_t i = 0; i < UniformBuffersVP.size(); i++) {
		UniformBuffersVP[i].Destroy(Globals::gDevice.Get());
	}

	mVertexBuffer.Destroy(Globals::gDevice.Get());
	mIndexBuffer.Destroy(Globals::gDevice.Get());
}
