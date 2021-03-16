#include "Entities.h"

Engine::SpotlightObject::SpotlightObject() {
	spotlightSettings.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	spotlightSettings.lightPosition = glm::vec3(1.0f, 12.0f, 1.0f);
	Type = ENTITY_TYPE_SPOTLIGHT_OBJECT;
	Name = "Spotlight";
}

void Engine::SpotlightObject::UpdateUniforms(uint32_t imageIndex, VkDevice device) {
	spotlightSettings.lightPosition = Transform.GetPosition();
}

Engine::DataTypes::SpotlightAttributes_t* Engine::SpotlightObject::pGetSpotlightUniformData() {
	return &spotlightSettings;
}

void Engine::SpotlightObject::Destroy() {

}

Engine::Entity::Entity() {
	ID = 0;
	Name = "Entity";
}

void Engine::Entity::Draw(VkCommandBuffer commandBuffer, int imageIndex) {

}

void Engine::Entity::Start() {

}

void Engine::Entity::Update() {

}

void Engine::Entity::SetName(std::string name) {
	Name = name;
}

void Engine::Entity::SetID(int Id) {
	ID = Id;
}

Engine::EntityType Engine::Entity::GetEntityType() {
	return Type;
}

std::string Engine::Entity::GetName() {
	return Name;
}

int Engine::Entity::GetID() {
	return ID;
}

Engine::GameObject::GameObject() {
	Type = ENTITY_TYPE_GAME_OBJECT;
	Name = "Game Object";
	ID = 0;
}

void Engine::GameObject::UpdateUniforms(uint32_t imageIndex, VkDevice device, std::vector<DataTypes::SpotlightAttributes_t*> spotlightAttributes) {
	if (pRigidBody != nullptr) {
		pRigidBody->pGetDebugMesh()->UpdateUniforms(imageIndex, device);
	}
	if (pMesh != nullptr) {
		pMesh->UpdateUniforms(imageIndex, device, Transform.GetMatrixProduct(), spotlightAttributes);
	}
}

void Engine::GameObject::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
	if (ENABLE_RIGIDBODY_MESH && Globals::gShowRigidbodyMeshes && pRigidBody != nullptr) {
		pRigidBody->pGetDebugMesh()->Draw(commandBuffer, imageIndex);
	}
	if (Globals::gShowMeshes && pMesh != nullptr) {
		pMesh->Draw(commandBuffer, imageIndex);
	}
}

void Engine::GameObject::Destroy() {
	if (pMesh != nullptr) {
		pMesh->Destroy();
		delete pMesh;
	}
	if (pRigidBody != nullptr) {
		pRigidBody->Destroy(Globals::gDynamicsWorld);
		delete pRigidBody;
	}
}

void Engine::GameObject::ApplyPhysicsToEntity() {
	if (pRigidBody != 0) {
		if (pRigidBody->GetBulletRigidBody()->getMass() != 0) {
			btTransform worldTransform;
			worldTransform = pRigidBody->GetBulletRigidBody()->getCenterOfMassTransform();

			glm::vec3 position = glm::vec3(worldTransform.getOrigin().getX(),
				worldTransform.getOrigin().getY(),
				worldTransform.getOrigin().getZ());

			Transform.Translate(glm::vec3(position.x, position.y, position.z));

			glm::quat quaternion = glm::quat(
				(float)worldTransform.getRotation().getX(),
				(float)worldTransform.getRotation().getY(),
				(float)worldTransform.getRotation().getZ(),
				(float)worldTransform.getRotation().getW()
			);

			Transform.SetQuaternion(quaternion);

			if (pRigidBody->pGetDebugMesh() != nullptr) {
				pRigidBody->pGetDebugMesh()->Transform.Translate(position);
				pRigidBody->pGetDebugMesh()->Transform.SetQuaternion(quaternion);
			}
		}
	}
}

void Engine::GameObject::ApplyEntityTransformToRigidbody() {
	if (pRigidBody != nullptr) {
		pRigidBody->SetRigidBodyTransform(Transform);
	}
}

Engine::CubemapObject::CubemapObject(std::vector<std::string> paths) {
	Type = ENTITY_TYPE_CUBEMAP_OBJECT;
	Name = "CubemapObect";
	ID = 0;

	pMesh = new CubemapMesh;
	pMesh->CreateCubemapMesh(paths);
}

void Engine::CubemapObject::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
	if (Globals::gShowSkybox && pMesh != nullptr) {
		pMesh->Draw(commandBuffer, imageIndex);
	}
}

void Engine::CubemapObject::UpdateUniforms(uint32_t imageIndex, VkDevice device) {
	if (pMesh != nullptr) {
		pMesh->UpdateUniforms(imageIndex, device);
	}
}

void Engine::CubemapObject::Destroy() {
	if (pMesh != nullptr) {
		pMesh->Destroy();
		delete pMesh;
	}
}
