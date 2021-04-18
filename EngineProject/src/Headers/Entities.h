#pragma once
#include "../../vendor/volk.h"
#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/quaternion.hpp"
#include "../../vendor/glm/gtx/quaternion.hpp"

#include "Renderer/DescriptorSetLayouts.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Images.h"
#include "Renderer/Buffers.h"

#include "RigidBody.h"
#include "Transform.h"
#include "Mesh.h"
//#include "Camera.h"

#include <GLFW/glfw3.h>
#include <vector>

namespace Engine{
	enum EntityType{
		ENTITY_TYPE_GAME_OBJECT,
		ENTITY_TYPE_CUBEMAP_OBJECT,
		ENTITY_TYPE_POINTLIGHT_OBJECT,
		ENTITY_TYPE_DIRECTIONAL_LIGHT_OBJECT
	};

	enum ComponentType {
		COMPONENT_TYPE_RIGIDBODY,
		COMPONENT_TYPE_MESH
	};

	class Entity {  
		protected :
			EntityType Type;
			int ID;
			std::string Name;
		public :
		Entity();
		virtual ~Entity();

		Transform Transform;

		virtual void Draw(VkCommandBuffer commandBuffer, int imageIndex);

		//virtual void Destroy()=0;

		virtual void Start();
		
		virtual void Update();

		void SetName(std::string name);

		void SetID(int Id);

		EntityType GetEntityType();

		std::string GetName();

		int GetID();
	};

    class Camera {
    protected:
        glm::vec3 CameraPos;
        glm::vec3 CameraUp;
        glm::vec3 CameraFront;
        glm::mat4 ProjectionMatrix;
        float     FOV;
        double    Yaw,
            Pitch,
            CursorLastX,
            CursorLastY;
    public:
        bool	  CursorFirstMouse;

        Camera();

		DataTypes::ViewProjection_t GetViewProjectionForEntity();
		
		DataTypes::ViewProjection_t GetViewProjectionForCubemap();

        glm::vec3 GetPosition();

        virtual void Update();

    };

    class EditorCamera : public Camera {

        const float moveSpeed = 5.25f;

        const float sprintSpeed = 2.f;

        void MouseUpdate();
    public:
        EditorCamera();

        void Update();
    };

	class DirectionalLightObject : public Entity{
        DataTypes::DirectionalLightAttributes_t directionalLightSettings;
        Mesh* pDebugMesh;

    public:

		DirectionalLightObject();
		~DirectionalLightObject();

		void UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera);;

		DataTypes::DirectionalLightAttributes_t* pGetDirectionalLightUniformData();;

		
	};

	class PointLightObject : public Entity {
		DataTypes::PointLightAttributes_t pointLightSettings;
		Mesh	  *pDebugMesh;
	public:
		PointLightObject();
		~PointLightObject();

		void UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera);

		DataTypes::PointLightAttributes_t *pGetPointLightUniformData();	
	};

	class GameObject : public Entity{

        RigidBody* pRigidBody;
        Mesh* pMesh;
		
		public:
	
		GameObject();
		~GameObject();

		/*void AddComponent(ComponentType Type) {
			if (Type == ComponentType::COMPONENT_TYPE_RIGIDBODY){
				if (pRigidBody == nullptr) pRigidBody = new RigidBody;
				else {
					delete pRigidBody;
					pRigidBody = new RigidBody;
				}
			}
			if (Type == ComponentType::COMPONENT_TYPE_MESH){
                if (pMesh == nullptr) pMesh = new Mesh;
				else {
                    delete pMesh;
					pMesh = new Mesh;
				}
			}
			else {
				spdlog::warn("Invalid type!");
			}
		}*/

        template <typename T> void AddComponent() {

        }

		template <> void AddComponent<RigidBody>() {
            if (pRigidBody == nullptr) pRigidBody = new RigidBody;
            else {
                delete pRigidBody;
                pRigidBody = new RigidBody;
            }
		}

        template <> void AddComponent<Mesh>() {
            if (pMesh == nullptr) pMesh = new Mesh;
            else {
                delete pMesh;
                pMesh = new Mesh;
            }
        }

        template <typename T> T pGetComponent() {

        }

        template <> RigidBody* pGetComponent<RigidBody*>() {
			return pRigidBody;
        }

        template <> Mesh* pGetComponent<Mesh*>() {
            return pMesh;
        }

		/*void* GetComponent(ComponentType Type) {
			if (Type == ComponentType::COMPONENT_TYPE_RIGIDBODY) {
				return pRigidBody;
			}
			if (Type == ComponentType::COMPONENT_TYPE_MESH){
				return pMesh;
			}
			else {
				spdlog::warn("Invalid type!");
				return 0;
			}
		}*/


		void UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera, std::vector<DataTypes::PointLightAttributes_t*> spotlightAttributes, std::vector<DataTypes::DirectionalLightAttributes_t*> directionalLightAttributes);

		void Draw(VkCommandBuffer commandBuffer, int imageIndex) override;

		void ApplyPhysicsToEntity();
	
		void ApplyEntityTransformToRigidbody();
		
	};

	class CubemapObject : public Entity{
		CubemapMesh *pMesh;
		std::vector<std::string> cubemapPaths;
		public:
			
		CubemapObject(std::vector<std::string> paths);
		~CubemapObject();

		void Draw(VkCommandBuffer commandBuffer, int imageIndex);
		
		void UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera);

		std::vector<std::string> GetCubemapPaths();
	
	};

}
