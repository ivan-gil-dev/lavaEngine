#pragma once
#include "../vendor/volk.h"
#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include "../vendor/glm/glm.hpp"

namespace Lava{
	const uint32_t WIDTH = 1366;
	const uint32_t HEIGHT = 768;
	const int MAX_FRAMES = 2;
	VkSampleCountFlagBits MSAAsamples = VK_SAMPLE_COUNT_2_BIT;
	VkSampleCountFlagBits maxMSAAsamples;
	//Индексы используемых очередей
	struct QueueIndices {
		uint32_t graphicsQueueIndex;
		uint32_t presentQueueIndex;
	};

	//структура с данными для каждой вершины
	struct Vertex{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::vec3 normals;
	};

	struct CubemapVertex {
		glm::vec3 pos;
	};

	struct UniformObjectMVP {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;

	};

	struct UniformObjectPointLightAttributes {
		glm::vec3 lightPosition;
		glm::vec3 lightColor;
	};

	struct UniformObjectLightColor {
		glm::vec3 lightColor;
	};

	struct UniformObjectCameraPos {
		glm::vec3 pos;
	};

	struct UniformObjectMaterial {
		float ambient;
		float diffuse;
		float specular;
	};

	struct UniformObjectCubemapMp{
		glm::mat4 model;
		glm::mat4 projection;
	};

	struct UniformObjectId{
		int id;
	};
}

