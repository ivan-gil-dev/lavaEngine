#ifndef datatypes_h
#define datatypes_h

#include "../Globals.h"

namespace Engine {
    namespace DataTypes {
        //Индексы очередей//
        struct QueueIndices_t {
            uint32_t graphicsQueueIndex;
            uint32_t presentQueueIndex;
        };

        //Вершина трехмерной модели//
        struct MeshVertex_t {
            glm::vec3 pos;//Координаты вершины//
            glm::vec3 color;//Цвет//
            glm::vec2 UVmap;//Координаты UV развертки//
            glm::vec3 normals;//Направление нормали//

            MeshVertex_t() {
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

        //Вершина контура физического тела//
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

        //Матрицы перемещения//
        struct MVP_t {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        //Произвидение матриц перемещения//
        //для источника света//
        struct LightSpace_t {
            glm::mat4 lightSpace;
        };

        //атрибуты направленого источника света//
        struct DirectionalLightAttributes_t {
            //Направление света//
            glm::vec3 lightDirection;
            //Цвет источника света//
            alignas(16) glm::vec3 lightColor;
            alignas(32) float ambient;
            float diffuse;
            float specular;
        };

        //атрибуты точечного источника света//
        struct PointLightAttributes_t {
            //координаты позиции источника света//
            glm::vec3 lightPosition;
            //цвет источника света//
            alignas(16) glm::vec3 lightColor;
            alignas(32)float ambient;

            float diffuse;
            float specular;

            float constant;
            float linear;
            float quadrantic;
        };

        //Координаты позиции камеры//
        struct CameraPos_t {
            glm::vec3 pos;
        };

        //Материал модели//
        struct Material_t {
            float shininess;
            float metallic;
            float roughness;
            float ao;
        };

        //Матрицы вида и проекции//
        struct ViewProjection_t {
            glm::mat4 view;
            glm::mat4 projection;
        };

        //константы для передачи без буферов//
        struct PushConstants
        {
            int diffuseMapId;
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