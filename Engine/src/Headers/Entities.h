#ifndef entities_h
#define entities_h

#include "../../vendor/volk.h"
#include "../../vendor/glm/glm.hpp"
#include "../../vendor/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/gtc/quaternion.hpp"
#include "../../vendor/glm/gtx/quaternion.hpp"

#include "DescriptorSetLayouts.h"
#include "DataTypes.h"
#include "Images.h"
#include "Buffers.h"
#include "Camera.h"
#include "RigidBody.h"
#include "Transform.h"
#include "Mesh.h"

#include <GLFW/glfw3.h>
#include <vector>

namespace Engine{
	enum EntityType{
		ENTITY_TYPE_GAME_OBJECT,
		ENTITY_TYPE_CUBEMAP_OBJECT,
		ENTITY_TYPE_SPOTLIGHT_OBJECT
	};

	class Entity {  
		protected :
			EntityType Type;
			int ID;
			std::string Name;
		public :
		Entity();

		Transform Transform;

		virtual void Draw(VkCommandBuffer commandBuffer, int imageIndex);;

		virtual void Destroy()=0;

		virtual void Start();;
		
		virtual void Update();;

		void SetName(std::string name);

		void SetID(int Id);

		EntityType GetEntityType();

		std::string GetName();

		int GetID();
	};

	class SpotlightObject : public Entity {
		DataTypes::SpotlightAttributes_t spotlightSettings;
		Mesh	  DebugMesh;
	public:
		SpotlightObject();
		
		void UpdateUniforms(uint32_t imageIndex, VkDevice device);

		DataTypes::SpotlightAttributes_t *pGetSpotlightUniformData();	
	
		void Destroy();
	};

	class GameObject : public Entity{
		public:
		RigidBody *pRigidBody;
		Mesh	  *pMesh;

		GameObject();

		void UpdateUniforms(uint32_t imageIndex, VkDevice device, std::vector<DataTypes::SpotlightAttributes_t*> spotlightAttributes);

		void Draw(VkCommandBuffer commandBuffer, int imageIndex) override;

		void Destroy() override;	

		void ApplyPhysicsToEntity();
	
		void ApplyEntityTransformToRigidbody();

	};

	class CubemapObject : public Entity{
		CubemapMesh *pMesh;
		public:
			
		CubemapObject(std::vector<std::string> paths);
		
		void Draw(VkCommandBuffer commandBuffer, int imageIndex);
		
		void UpdateUniforms(uint32_t imageIndex, VkDevice device);

		void Destroy();
	
	};

}
#endif