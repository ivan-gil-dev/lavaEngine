#include "Mesh.h"
#include <algorithm>
#include "Renderer/Renderer.h"
#define TINYOBJLOADER_STATIC
#include    "../../vendor/tiny_obj_loader.h"

void Engine::Mesh::LoadModel(std::string modelPath) {
    std::vector<DataTypes::MeshVertex_t>	Vertices;
    
	size_t lastPos = modelPath.find_last_of("/");

	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = modelPath.substr(0, lastPos+1); // Path to material files
	
	std::cout << modelPath.substr(0, lastPos+1) << std::endl;


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

	bool MaterialsFound = true;
	if (materials.size() == 0)
	{
		MaterialsFound = false;
	}


	if (MaterialsFound)
	{
		Faces.resize(materials.size());
		for (size_t i = 0; i < Faces.size(); i++)
		{
			Faces[i].diffuseMapPath = reader_config.mtl_search_path + materials[i].diffuse_texname;
			Faces[i].MatID = i;
		}
	}
	else {
		Faces.resize(1);
		Faces[0].diffuseMapPath = "";
		Faces[0].MatID = 0;
	}

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

				if (MaterialsFound)
				{
					Faces[shapes[s].mesh.material_ids[f]].indexes.push_back(UniqueVertices[vertex]);
				}
				else {
					Faces[0].indexes.push_back(UniqueVertices[vertex]);
				}
				
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
			
			
		}
	}

	
	
    VertexBuffer.CreateVertexBuffer(renderer.physicalDevice.Get(),
        renderer.device.Get(),
        renderer.device.GetGraphicsQueue(),
        renderer.commandPool.Get(),
        Vertices.data(),
        sizeof(Vertices[0]) * Vertices.size());

	for (size_t i = 0; i < Faces.size(); i++)
	{
        Faces[i].indexBuffer.CreateIndexBuffer(renderer.physicalDevice.Get(),
            renderer.device.Get(),
            renderer.device.GetGraphicsQueue(),
            renderer.commandPool.Get(),
            Faces[i].indexes.data(), sizeof(Faces[i].indexes[0]) * Faces[i].indexes.size());
	}
       
}

void Engine::Mesh::CreateDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayoutForMesh, 
	VkDescriptorPool descriptorPoolForGameObjects, std::vector<VkImageView> swapchainImageViews) {

	std::vector<VkDescriptorSetLayout> layouts(swapchainImageViews.size(), descriptorSetLayoutForMesh);

	VkDescriptorSetAllocateInfo allocateInfo{};
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
        bufferInfo.buffer = UniformBuffersMVP_b0[i].Get();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(DataTypes::MVP_t);
		
		VkWriteDescriptorSet mvpWriteDescriptorSet{};
        mvpWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mvpWriteDescriptorSet.descriptorCount = 1;
        mvpWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpWriteDescriptorSet.dstSet = DescriptorSets[i];
        mvpWriteDescriptorSet.dstBinding = 0;
        mvpWriteDescriptorSet.dstArrayElement = 0;
        mvpWriteDescriptorSet.pBufferInfo = &bufferInfo;
        writeDescriptorSets.push_back(mvpWriteDescriptorSet);
		
		std::vector<VkDescriptorImageInfo> imageInfos;
		for (int j = 0; j < DiffuseTextures_b1.size(); j++)
		{
            VkDescriptorImageInfo textureInfo{};
            textureInfo.imageView = DiffuseTextures_b1[j].GetImageView();
            textureInfo.sampler = DiffuseTextures_b1[j].GetImageSampler();
            textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfos.push_back(textureInfo);
		}
		
        VkWriteDescriptorSet textureWriteDescriptorSet{};
        textureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        textureWriteDescriptorSet.descriptorCount = DiffuseTextures_b1.size();
        textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureWriteDescriptorSet.dstSet = DescriptorSets[i];
        textureWriteDescriptorSet.dstBinding = 1;
        textureWriteDescriptorSet.dstArrayElement = 0;
        textureWriteDescriptorSet.pImageInfo = imageInfos.data();

        writeDescriptorSets.push_back(textureWriteDescriptorSet);



		VkDescriptorBufferInfo bufferInfo2{};
        bufferInfo2.buffer = UniformBuffersSpotLightAttributes_b2[i].Get();
        bufferInfo2.offset = 0;
        bufferInfo2.range = sizeof(DataTypes::PointLightAttributes_t) * MAX_SPOTLIGHTS;
		
		VkWriteDescriptorSet spotLightAttributesWriteDescriptorSet{};
        spotLightAttributesWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        spotLightAttributesWriteDescriptorSet.descriptorCount = 1;
        spotLightAttributesWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        spotLightAttributesWriteDescriptorSet.dstSet = DescriptorSets[i];
        spotLightAttributesWriteDescriptorSet.dstBinding = 2;
        spotLightAttributesWriteDescriptorSet.dstArrayElement = 0;
        spotLightAttributesWriteDescriptorSet.pBufferInfo = &bufferInfo2;
        writeDescriptorSets.push_back(spotLightAttributesWriteDescriptorSet);
		
		VkDescriptorBufferInfo bufferInfo3{};
        bufferInfo3.buffer = UniformBuffersDebugCameraPos_b3[i].Get();
        bufferInfo3.offset = 0;
        bufferInfo3.range = sizeof(DataTypes::CameraPos_t);
		
		VkWriteDescriptorSet debugCameraPosWriteDescriptorSet{};
        debugCameraPosWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        debugCameraPosWriteDescriptorSet.descriptorCount = 1;
        debugCameraPosWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        debugCameraPosWriteDescriptorSet.dstSet = DescriptorSets[i];
        debugCameraPosWriteDescriptorSet.dstBinding = 3;
        debugCameraPosWriteDescriptorSet.dstArrayElement = 0;
        debugCameraPosWriteDescriptorSet.pBufferInfo = &bufferInfo3;
        writeDescriptorSets.push_back(debugCameraPosWriteDescriptorSet);
		
		VkDescriptorBufferInfo bufferInfo4{};
        bufferInfo4.buffer = UniformBuffersMaterial_b4[i].Get();
        bufferInfo4.offset = 0;
        bufferInfo4.range = sizeof(DataTypes::Material_t);
		
		VkWriteDescriptorSet materialWriteDescriptorSet{};
		materialWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		materialWriteDescriptorSet.descriptorCount = 1;
		materialWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		materialWriteDescriptorSet.dstSet = DescriptorSets[i];
		materialWriteDescriptorSet.dstBinding = 4;
		materialWriteDescriptorSet.dstArrayElement = 0;
		materialWriteDescriptorSet.pBufferInfo = &bufferInfo4;
		writeDescriptorSets.push_back(materialWriteDescriptorSet);

        VkDescriptorBufferInfo bufferInfo5{};
		bufferInfo5.buffer = UniformBuffersDirectionalLightAttributes_b5[i].Get();
		bufferInfo5.offset = 0;
		bufferInfo5.range = sizeof(DataTypes::DirectionalLightAttributes_t) * MAX_DLIGHTS;

        VkWriteDescriptorSet DirectionalLightWriteDescriptorSet{};
		DirectionalLightWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DirectionalLightWriteDescriptorSet.descriptorCount = 1;
		DirectionalLightWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		DirectionalLightWriteDescriptorSet.dstSet = DescriptorSets[i];
		DirectionalLightWriteDescriptorSet.dstBinding = 5;
		DirectionalLightWriteDescriptorSet.dstArrayElement = 0;
		DirectionalLightWriteDescriptorSet.pBufferInfo = &bufferInfo5;
        writeDescriptorSets.push_back(DirectionalLightWriteDescriptorSet);
		
		vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		writeDescriptorSets.resize(0);
	}

    
}

//VkBuffer Engine::Mesh::GetVertexBuffer() {
//	return VertexBuffer.Get();
//}
//
//VkBuffer Engine::Mesh::GetIndexBuffer() {
//	//return IndexBuffer.Get();
//	return 0;
//}

std::string Engine::Mesh::pGetMeshPath() {
	return MeshPath;
}

std::vector<Engine::DataTypes::MeshVertex_t>* Engine::Mesh::GetVertices() {
    return &Vertices;
}

//std::vector<glm::uint32_t> Engine::Mesh::GetIndexes() {
//	return Indexes;
//}

Engine::DataTypes::Material_t Engine::Mesh::GetMaterial() {
	return Material;
}

void Engine::Mesh::SetDiffuseTexture(std::string path, short id) {
	if (id < DiffuseMapsSize && id >= 0){
        DiffuseTextures_b1[id].DestroyTexture(renderer.device.Get());
        vkFreeDescriptorSets(renderer.device.Get(), renderer.descriptorPoolForMesh.Get(),
            (uint32_t)DescriptorSets.size(), DescriptorSets.data());
        DiffuseTextures_b1[id].CreateTexture(
            renderer.physicalDevice.Get(), renderer.device.Get(), renderer.device.GetGraphicsQueue(), renderer.commandPool.Get(), path
        );
        CreateDescriptorSets(renderer.device.Get(), renderer.setLayoutForMesh.Get(),
            renderer.descriptorPoolForMesh.Get(), *renderer.swapchain.PGetImageViews());
	}
	
}

Engine::Texture Engine::Mesh::GetDiffuseTexture(short id){
	return DiffuseTextures_b1[id];
}

void Engine::Mesh::SetMaterial(DataTypes::Material_t mat) {
	Material = mat;
}

void Engine::Mesh::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		renderer.graphicsPipelineForMesh.Get()
	);

	if (ENABLE_DYNAMIC_VIEWPORT) {
		vkCmdSetViewport(commandBuffer, 0, 1, &renderer.rendererViewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &renderer.rendererScissors);
	}

	VkBuffer buffers[] = { VertexBuffer.Get() };
	VkDeviceSize offsets[] = { 0 };


	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.graphicsPipelineForMesh.GetPipelineLayout(),
		0, 1, &DescriptorSets[imageIndex], 0, nullptr);

	for (size_t i = 0; i < Faces.size(); i++)
	{
		DataTypes::PushConstants constants;
		constants.diffuseMapId = Faces[i].MatID;

		vkCmdPushConstants(commandBuffer, renderer.graphicsPipelineForMesh.GetPipelineLayout(),
			VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), &constants);

        vkCmdBindIndexBuffer(commandBuffer, Faces[i].indexBuffer.Get(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, (uint32_t)Faces[i].indexes.size(), 1, 0, 0, 0);
	}
	
}

void Engine::Mesh::CreateMesh(std::string modelPath) {
	Material = { 32.f };
	MeshPath = modelPath;

	LoadModel(modelPath);

	{//Создание буферов
		
        //<1x1 текстура (0,0,0,255)> 

		DiffuseTextures_b1.resize(DiffuseMapsSize);

		for (size_t i = 0; i < DiffuseTextures_b1.size(); i++)
		{
			if (i < Faces.size())
                DiffuseTextures_b1[i].CreateTexture(renderer.physicalDevice.Get(),
                    renderer.device.Get(),
                    renderer.device.GetGraphicsQueue(),
                    renderer.commandPool.Get(), Faces[i].diffuseMapPath);
			else
				DiffuseTextures_b1[i].CreateTexture(renderer.physicalDevice.Get(),
                    renderer.device.Get(),
                    renderer.device.GetGraphicsQueue(),
                    renderer.commandPool.Get(), "");
		}

		UniformBuffersMVP_b0.resize(renderer.swapchain.PGetImageViews()->size());
		UniformBuffersSpotLightAttributes_b2.resize(renderer.swapchain.PGetImageViews()->size());
		UniformBuffersDebugCameraPos_b3.resize(renderer.swapchain.PGetImageViews()->size());
		UniformBuffersMaterial_b4.resize(renderer.swapchain.PGetImageViews()->size());
		UniformBuffersDirectionalLightAttributes_b5.resize(renderer.swapchain.PGetImageViews()->size());

		for (size_t i = 0; i < renderer.swapchain.PGetImageViews()->size(); i++) {
			UniformBuffersMVP_b0[i].CreateUniformBuffer(renderer.physicalDevice.Get(), renderer.device.Get(), sizeof(DataTypes::MVP_t));
			UniformBuffersDebugCameraPos_b3[i].CreateUniformBuffer(renderer.physicalDevice.Get(), renderer.device.Get(), sizeof(DataTypes::CameraPos_t));
			UniformBuffersMaterial_b4[i].CreateUniformBuffer(renderer.physicalDevice.Get(), renderer.device.Get(), sizeof(DataTypes::Material_t));
			UniformBuffersSpotLightAttributes_b2[i].CreateUniformBuffer(renderer.physicalDevice.Get(), renderer.device.Get(),
				sizeof(DataTypes::PointLightAttributes_t) * MAX_SPOTLIGHTS);
			UniformBuffersDirectionalLightAttributes_b5[i].CreateUniformBuffer(renderer.physicalDevice.Get(), renderer.device.Get(), 
				sizeof(DataTypes::DirectionalLightAttributes_t)*MAX_DLIGHTS);
		}
	}

	CreateDescriptorSets(
		renderer.device.Get(), renderer.setLayoutForMesh.Get(),
		renderer.descriptorPoolForMesh.Get(), *renderer.swapchain.PGetImageViews()
	);
}

void Engine::Mesh::UpdateUniforms(uint32_t imageIndex, VkDevice device, glm::vec3 cameraPosition, 
	DataTypes::ViewProjection_t viewProjection, glm::mat4 TransformMatrixProduct, std::vector<DataTypes::PointLightAttributes_t*> spotlightAttributes, 
	std::vector <DataTypes::DirectionalLightAttributes_t*> directionalLightAttributes) {
	
	MVP.model = TransformMatrixProduct;
	MVP.view = viewProjection.view;
	MVP.proj = viewProjection.projection;

	MVP.proj[1][1] *= -1;

	void* data;
	vkMapMemory(device, UniformBuffersMVP_b0[imageIndex].GetDeviceMemory(), 0, sizeof(MVP), 0, &data);
	memcpy(data, &MVP, sizeof(MVP));
	vkUnmapMemory(device, UniformBuffersMVP_b0[imageIndex].GetDeviceMemory());

	vkMapMemory(device, UniformBuffersMaterial_b4[imageIndex].GetDeviceMemory(), 0, sizeof(DataTypes::Material_t), 0, &data);
	memcpy(data, &Material, sizeof(DataTypes::Material_t));
	vkUnmapMemory(device, UniformBuffersMaterial_b4[imageIndex].GetDeviceMemory());

	std::vector<DataTypes::PointLightAttributes_t> buffer_SpotlightAttributes(MAX_SPOTLIGHTS);

	for (size_t i = 0; i < buffer_SpotlightAttributes.size(); i++) {
		if (i < spotlightAttributes.size()){
			buffer_SpotlightAttributes[i] = *spotlightAttributes[i];
		}

		else {
			buffer_SpotlightAttributes[i].lightColor = glm::vec3(0, 0, 0);
			buffer_SpotlightAttributes[i].lightPosition = glm::vec3(0, 0, 0);
			buffer_SpotlightAttributes[i].constant = 0.0f;
			buffer_SpotlightAttributes[i].linear = 0.0f;
			buffer_SpotlightAttributes[i].quadrantic = 0.0f;
		}
	}

	vkMapMemory(device, UniformBuffersSpotLightAttributes_b2[imageIndex].GetDeviceMemory(), 0,
		sizeof(DataTypes::PointLightAttributes_t) * buffer_SpotlightAttributes.size(), 0, &data);
	memcpy(data, buffer_SpotlightAttributes.data(), sizeof(DataTypes::PointLightAttributes_t) * buffer_SpotlightAttributes.size());
	vkUnmapMemory(device, UniformBuffersSpotLightAttributes_b2[imageIndex].GetDeviceMemory());

	buffer_SpotlightAttributes.clear();


	DataTypes::CameraPos_t cameraPos;
	cameraPos.pos = cameraPosition;

	vkMapMemory(device, UniformBuffersDebugCameraPos_b3[imageIndex].GetDeviceMemory(), 0, sizeof(DataTypes::CameraPos_t), 0, &data);
	memcpy(data, &cameraPos, sizeof(DataTypes::CameraPos_t));
    vkUnmapMemory(device, UniformBuffersDebugCameraPos_b3[imageIndex].GetDeviceMemory());

	std::vector<DataTypes::DirectionalLightAttributes_t> buffer_DirectionalLightAttributes(MAX_DLIGHTS);
    for (size_t i = 0; i < buffer_DirectionalLightAttributes.size(); i++) {
        if (i < directionalLightAttributes.size()) {
			buffer_DirectionalLightAttributes[i] = *directionalLightAttributes[i];
        }

        else {
			buffer_DirectionalLightAttributes[i].lightColor = glm::vec3(0, 0, 0);
			buffer_DirectionalLightAttributes[i].lightDirection = glm::vec3(0, 0, 0);
        }
    }
	//spdlog::info("Debug 1: {:6.2f}", buffer_DirectionalLightAttributes[0].lightColor.r);
	vkMapMemory(device, UniformBuffersDirectionalLightAttributes_b5[imageIndex].GetDeviceMemory(), 0, 
		buffer_DirectionalLightAttributes.size()*sizeof(DataTypes::DirectionalLightAttributes_t) , 0, &data);
    memcpy(data, buffer_DirectionalLightAttributes.data(), buffer_DirectionalLightAttributes.size() * sizeof(DataTypes::DirectionalLightAttributes_t));
    vkUnmapMemory(device, UniformBuffersDirectionalLightAttributes_b5[imageIndex].GetDeviceMemory());

	buffer_DirectionalLightAttributes.clear();
}

void Engine::Mesh::Destroy() {
	for (size_t i = 0; i < DiffuseTextures_b1.size(); i++)
	{
		DiffuseTextures_b1[i].DestroyTexture(renderer.device.Get());
	}
	

	vkFreeDescriptorSets(renderer.device.Get(), renderer.descriptorPoolForMesh.Get(),
		(uint32_t)DescriptorSets.size(), DescriptorSets.data());

	for (size_t i = 0; i < UniformBuffersMVP_b0.size(); i++) {
		UniformBuffersMVP_b0[i].Destroy(renderer.device.Get());
		UniformBuffersDebugCameraPos_b3[i].Destroy(renderer.device.Get());
		UniformBuffersMaterial_b4[i].Destroy(renderer.device.Get());
		UniformBuffersDirectionalLightAttributes_b5[i].Destroy(renderer.device.Get());
		UniformBuffersSpotLightAttributes_b2[i].Destroy(renderer.device.Get());

	}
	VertexBuffer.Destroy(renderer.device.Get());
	for (size_t i = 0; i < Faces.size(); i++)
	{
		Faces[i].indexBuffer.Destroy(renderer.device.Get());
	}
	
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
		renderer.gGraphicsPipelineForRigidBodyMesh.Get()
	);

	VkBuffer buffers[] = { VertexBuffer.Get() };

	VkDeviceSize offsets[] = { 0 };

	if (ENABLE_DYNAMIC_VIEWPORT) {
		vkCmdSetViewport(commandBuffer, 0, 1, &renderer.rendererViewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &renderer.rendererScissors);
	}

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.gGraphicsPipelineForRigidBodyMesh.GetPipelineLayout(),
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

	VertexBuffer.CreateVertexBuffer(renderer.physicalDevice.Get(), renderer.device.Get(),
		renderer.device.GetGraphicsQueue(), renderer.commandPool.Get(),
		Vertices.data(), sizeof(Vertices[0]) * Vertices.size());

	IndexBuffer.CreateIndexBuffer(renderer.physicalDevice.Get(), renderer.device.Get(),
		renderer.device.GetGraphicsQueue(), renderer.commandPool.Get(),
		Indexes.data(), sizeof(Indexes[0]) * Indexes.size());

	UniformBuffersMVP.resize(renderer.swapchain.PGetImageViews()->size());

	for (size_t i = 0; i < renderer.swapchain.PGetImageViews()->size(); i++) {
		UniformBuffersMVP[i].CreateUniformBuffer(renderer.physicalDevice.Get(), renderer.device.Get(), sizeof(DataTypes::MVP_t));
	}

	CreateDescriptorSets(renderer.device.Get(), renderer.setLayoutForRigidBodyMesh.Get(),
		renderer.descriptorPoolForMesh.Get(), *renderer.swapchain.PGetImageViews());
}

void Engine::WireframeMesh::UpdateUniforms(uint32_t imageIndex, VkDevice device, DataTypes::ViewProjection_t viewProjection) {

	MVP.model = Transform.GetMatrixProduct();
	MVP.view = viewProjection.view;
	MVP.proj = viewProjection.projection;

	MVP.proj[1][1] *= -1;
	void* data;
	vkMapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory(), 0, sizeof(MVP), 0, &data);
	memcpy(data, &MVP, sizeof(MVP));
	vkUnmapMemory(device, UniformBuffersMVP[imageIndex].GetDeviceMemory());
}

void Engine::WireframeMesh::Destroy() {
	vkFreeDescriptorSets(renderer.device.Get(), renderer.descriptorPoolForMesh.Get(), 
		(uint32_t)DescriptorSets.size(), DescriptorSets.data());
	for (size_t i = 0; i < UniformBuffersMVP.size(); i++) {
		UniformBuffersMVP[i].Destroy(renderer.device.Get());
	}
	VertexBuffer.Destroy(renderer.device.Get());
	IndexBuffer.Destroy(renderer.device.Get());
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
		bufferInfo.range = sizeof(DataTypes::ViewProjection_t);

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
		renderer.device.Get(),
		renderer.physicalDevice.Get(),
		renderer.commandPool.Get(),
		renderer.device.GetGraphicsQueue(),
		paths
	);

	UniformBuffersVP.resize(renderer.swapchain.PGetImageViews()->size());

	for (size_t i = 0; i < renderer.swapchain.PGetImageViews()->size(); i++) {
		UniformBuffersVP[i].CreateUniformBuffer(
			renderer.physicalDevice.Get(),
			renderer.device.Get(),
			sizeof(DataTypes::ViewProjection_t)
		);
	}

	CreateDescriptorSets(
		renderer.device.Get(),
		renderer.setLayoutForCubemapObjects.Get(),
		renderer.descriptorPoolForCubemapObjects.Get(),
		*renderer.swapchain.PGetImageViews()
	);

	mVertexBuffer.CreateVertexBuffer(
		renderer.physicalDevice.Get(),
		renderer.device.Get(),
		renderer.device.GetGraphicsQueue(),
		renderer.commandPool.Get(),
		CubeMapVertices.data(),
		sizeof(CubeMapVertices[0]) * CubeMapVertices.size());

	mIndexBuffer.CreateIndexBuffer(
		renderer.physicalDevice.Get(),
		renderer.device.Get(),
		renderer.device.GetGraphicsQueue(),
		renderer.commandPool.Get(),
		Indexes.data(),
		sizeof(Indexes[0]) * Indexes.size());
}

void Engine::CubemapMesh::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		renderer.graphicsPipelineForCubemapObjects.Get()
	);

	if (ENABLE_DYNAMIC_VIEWPORT) {
		vkCmdSetViewport(commandBuffer, 0, 1, &renderer.rendererViewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &renderer.rendererScissors);
	}

	VkBuffer buffers[] = { mVertexBuffer.Get() };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.graphicsPipelineForCubemapObjects.GetPipelineLayout(),
		0, 1, &DescriptorSets[imageIndex], 0, nullptr);
	vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer.Get(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, (uint32_t)Indexes.size(), 1, 0, 0, 0);
}

void Engine::CubemapMesh::UpdateUniforms(uint32_t imageIndex, VkDevice device, DataTypes::ViewProjection_t viewProjection) {
	DataTypes::ViewProjection_t cubemapMp;
	cubemapMp.view = viewProjection.view;
	cubemapMp.projection = viewProjection.projection;
	cubemapMp.view[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	void* data;
	vkMapMemory(device, UniformBuffersVP[imageIndex].GetDeviceMemory(), 0, sizeof(cubemapMp), 0, &data);
	memcpy(data, &cubemapMp, sizeof(cubemapMp));
	vkUnmapMemory(device, UniformBuffersVP[imageIndex].GetDeviceMemory());
}

void Engine::CubemapMesh::Destroy() {
	mCubemapTexture.DestroyTexture(renderer.device.Get());
	vkFreeDescriptorSets(renderer.device.Get(),
		renderer.descriptorPoolForCubemapObjects.Get(),
		(uint32_t)DescriptorSets.size(),
		DescriptorSets.data());

	for (size_t i = 0; i < UniformBuffersVP.size(); i++) {
		UniformBuffersVP[i].Destroy(renderer.device.Get());
	}

	mVertexBuffer.Destroy(renderer.device.Get());
	mIndexBuffer.Destroy(renderer.device.Get());
}
