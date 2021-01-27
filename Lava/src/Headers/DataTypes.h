#pragma once

#include "Globals.h"

namespace Lava{
	namespace DataTypes{
		struct QueueIndices {
			uint32_t graphicsQueueIndex;
			uint32_t presentQueueIndex;
		};

		struct Vertex {
			bool operator==(const Vertex& other) const {
				return pos == other.pos && color == other.color && UVmap == other.UVmap && normals == other.normals;
			}
			glm::vec3 pos;
			glm::vec3 color;
			glm::vec2 UVmap;
			glm::vec3 normals;
		};

		struct WireframeVertex {
			bool operator==(const WireframeVertex& other) const {
				return pos == other.pos && color == other.color;
			}
			glm::vec3 pos;
			glm::vec3 color;
		};

		struct CubemapVertex {
			glm::vec3 pos;
		};

		struct U_Struct_MVP {
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		struct U_Struct_SpotlightAttributes {
			glm::vec3 lightPosition;
			glm::vec3 lightColor;
		};

		struct U_Struct_LightColor {
			glm::vec3 lightColor;
		};

		struct U_Struct_DebugCameraPos {
			glm::vec3 pos;
		};

		struct U_Struct_Material {
			float ambient;
			float diffuse;
			float specular;
		};

		struct U_Struct_CubemapVP {
			glm::mat4 view;
			glm::mat4 projection;
		};

		struct U_Struct_ID {
			int id;
		};
	
		
	}
}

template<> struct std::hash<Lava::DataTypes::Vertex> {
	size_t operator()(Lava::DataTypes::Vertex const& vertex) const {

		return ((std::hash<glm::vec3>()(vertex.pos) ^
				(std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(std::hash<glm::vec2>()(vertex.UVmap) << 1);

	}
};

template<> struct std::hash<Lava::DataTypes::WireframeVertex> {
	size_t operator()(Lava::DataTypes::WireframeVertex const& vertex) const {
		return ((std::hash<glm::vec3>()(vertex.pos) ^
				(std::hash<glm::vec3>()(vertex.color) << 1)) >> 1);
	}
};