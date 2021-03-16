#ifndef datatypes_h
#define datatypes_h

#include "Globals.h"

namespace Engine{
	namespace DataTypes{
		struct QueueIndices_t {
			uint32_t graphicsQueueIndex;
			uint32_t presentQueueIndex;
		};

		struct MeshVertex_t {
			glm::vec3 pos;
			glm::vec3 color;
			glm::vec2 UVmap;
			glm::vec3 normals;
			
			MeshVertex_t(){
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				color.x = 0;
				color.y = 0;
				color.z = 0;
				normals.x = 0;
				normals.y = 0;
				normals.z = 0;
				UVmap.x = 0;
				UVmap.y = 0;
			}

			bool operator==(const MeshVertex_t& other) const {
				return pos == other.pos && color == other.color && UVmap == other.UVmap && normals == other.normals;
			}

		};

		struct WireframeMeshVertex_t {
			bool operator==(const WireframeMeshVertex_t& other) const {
				return pos == other.pos && color == other.color;
			}
			glm::vec3 pos;
			glm::vec3 color;
			WireframeMeshVertex_t() {
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				color.x = 0;
				color.y = 0;
				color.z = 0;
			}
		};

		struct CubemapMeshVertex_t {
			glm::vec3 pos;
		};

		struct MVP_t {
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		struct SpotlightAttributes_t {
			glm::vec3 lightPosition;
			alignas(16) glm::vec3 lightColor;
		};

		struct CameraPos_t {
			glm::vec3 pos;
		};

		struct Material_t {
			float ambient;
			float diffuse;
			float specular;
		};

		struct CubemapVP_t {
			glm::mat4 view;
			glm::mat4 projection;
		};
	}
}

template<> struct std::hash<Engine::DataTypes::MeshVertex_t> {
	size_t operator()(Engine::DataTypes::MeshVertex_t const& vertex) const {

		return ((std::hash<glm::vec3>()(vertex.pos) ^
				(std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(std::hash<glm::vec2>()(vertex.UVmap) << 1);

	}
};

template<> struct std::hash<Engine::DataTypes::WireframeMeshVertex_t> {
	size_t operator()(Engine::DataTypes::WireframeMeshVertex_t const& vertex) const {
		return ((std::hash<glm::vec3>()(vertex.pos) ^
				(std::hash<glm::vec3>()(vertex.color) << 1)) >> 1);
	}
};

#endif