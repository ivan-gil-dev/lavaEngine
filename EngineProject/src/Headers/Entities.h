#  pragma warning( push )
#  pragma warning( disable: 4251 )


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

	class EngineAPI_Export Entity {
		protected :
			EntityType Type;
			int ID;
			std::string Name;
			Entity* ref;
		public :
		Entity();
		virtual ~Entity();

		Transform Transform;

		virtual void Draw(VkCommandBuffer commandBuffer, int imageIndex);

		//virtual void Destroy()=0;

		virtual void Start();
		
		virtual void Update();

		void SetRef(Entity* reference);

		void SetName(std::string name);

		void SetID(int Id);

		EntityType GetEntityType();

		std::string GetName();

		int GetID();
	};

    class EngineAPI_Export Camera : public Entity{
		
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
		bool Active = false;
    public:
        bool	  CursorFirstMouse;

        Camera();

		void SetCameraPos(glm::vec3 pos);

		void SetCameraFront(glm::vec3 front);

		virtual DataTypes::ViewProjection_t GetViewProjectionForEntity();
		
		virtual DataTypes::ViewProjection_t GetViewProjectionForCubemap();

        glm::vec3 GetPosition();

		glm::vec3 GetCameraFront();

		virtual void Update();

    };

    class EngineAPI_Export EditorCamera : public Camera {

        const float moveSpeed = 5.25f;

        const float sprintSpeed = 2.f;

        void MouseUpdate();
    public:
        EditorCamera();

		void Reset();

        void Update();
    };

	class EngineAPI_Export DirectionalLightObject : public Entity{
        DataTypes::DirectionalLightAttributes_t directionalLightSettings;
        Mesh* pDebugMesh;

    public:

		DirectionalLightObject();
		~DirectionalLightObject();

		void UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera);;

		DataTypes::DirectionalLightAttributes_t* pGetDirectionalLightUniformData();;

		
	};

	class EngineAPI_Export PointLightObject : public Entity {
		DataTypes::PointLightAttributes_t pointLightSettings;
		Mesh	  *pDebugMesh;
	public:
		PointLightObject();
		~PointLightObject();

		void UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera);

		DataTypes::PointLightAttributes_t *pGetPointLightUniformData();	
	};

	class EngineAPI_Export GameObject : public Entity{
		protected:
        RigidBody *pRigidBody;
		Mesh *pMesh;
		bool IsMeshCreated = false;
		bool IsRigidbodyCreated = false;

		public:
	
		GameObject();
		~GameObject();

		template <typename T> void AddComponent();

		template <> void AddComponent<RigidBody>();

		template <> void AddComponent<Mesh>();

		template <typename T> T pGetComponent();

		template <> RigidBody* pGetComponent<RigidBody*>();

		template <> Mesh* pGetComponent<Mesh*>();

		template <typename T> void DeleteComponent();

		template <> void DeleteComponent<RigidBody>();

		template <> void DeleteComponent<Mesh>();




		void UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera, std::vector<DataTypes::PointLightAttributes_t*> spotlightAttributes, std::vector<DataTypes::DirectionalLightAttributes_t*> directionalLightAttributes);

		void Draw(VkCommandBuffer commandBuffer, int imageIndex) override;

		void DrawShadowMaps(VkCommandBuffer commandBuffer, int imageIndex, std::vector<VkDescriptorSet>& pDescriptorSets);

		void ApplyPhysicsToEntity();
	
		void ApplyEntityTransformToRigidbody();
		
	};

	class EngineAPI_Export CubemapObject : public Entity{
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
#  pragma warning( pop )