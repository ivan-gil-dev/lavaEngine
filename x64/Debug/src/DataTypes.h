#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#define DEBUG
#include	<btBulletDynamicsCommon.h>

#include <../vendor/glm/gtx/hash.hpp>
#include	"../vendor/volk.h"
#include	"../vendor/glm/glm.hpp"
#include	<spdlog/spdlog.h>
#include	<GLFW/glfw3.h>
#include	<vector>
#include	<iostream>

namespace Lava{
	static bool  rigidBodyWireframeMesh = true;
	const uint32_t WIDTH = 1366;
	const uint32_t HEIGHT = 768;
	const int MAX_FRAMES = 2;
	static VkSampleCountFlagBits MSAAsamples = VK_SAMPLE_COUNT_2_BIT;
	static VkSampleCountFlagBits maxMSAAsamples;

	static btBroadphaseInterface* broadphase;
	static btDefaultCollisionConfiguration* collisionConfiguration;
	static btCollisionDispatcher* dispatcher;
	static btSequentialImpulseConstraintSolver* solver;
	static btDynamicsWorld* dynamicsWorld;


	//Индексы используемых очередей
	struct QueueIndices {
		uint32_t graphicsQueueIndex;
		uint32_t presentQueueIndex;
	};

	//структура с данными для каждой вершины
	struct Vertex{
		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord && normals == other.normals;
		}
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::vec3 normals;
	};
	
	struct WireframeVertex{
		bool operator==(const WireframeVertex& other) const {
			return pos == other.pos && color == other.color;
		}
		glm::vec3 pos;
		glm::vec3 color;
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

template<> struct std::hash<Lava::Vertex> {
	size_t operator()(Lava::Vertex const& vertex) const {
		return ((std::hash<glm::vec3>()(vertex.pos) ^
			(std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
			(std::hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};

template<> struct std::hash<Lava::WireframeVertex> {
	size_t operator()(Lava::WireframeVertex const& vertex) const {
		return ((std::hash<glm::vec3>()(vertex.pos) ^
			(std::hash<glm::vec3>()(vertex.color) << 1)) >> 1);
	}
};