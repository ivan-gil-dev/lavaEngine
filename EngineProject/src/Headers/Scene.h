#ifndef scene_h
#define scene_h



#include "Globals.h"
#include "Renderer/DataTypes.h"
#include <vector>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include "Entities.h"
namespace Engine{

	static void InitBullet() {
		Globals::gBroadphase = new btDbvtBroadphase();
		Globals::gCollisionConfiguration = new btDefaultCollisionConfiguration();
		Globals::gDispatcher = new btCollisionDispatcher(Globals::gCollisionConfiguration);
		Globals::gSolver = new btSequentialImpulseConstraintSolver;
		Globals::gDynamicsWorld = new btDiscreteDynamicsWorld(
			Globals::gDispatcher, Globals::gBroadphase, 
			Globals::gSolver, Globals::gCollisionConfiguration
		);
		Globals::gDynamicsWorld->setGravity(btVector3(0, -9.8, 0));
	}

	static void CleanBullet() {
		delete Globals::gSolver;
		delete Globals::gDispatcher;
		delete Globals::gCollisionConfiguration;
		delete Globals::gBroadphase;
		delete Globals::gDynamicsWorld;
	}

	class Scene {
		std::vector<DataTypes::DirectionalLightAttributes_t*> directionalLightAttributes;

		std::vector<DataTypes::PointLightAttributes_t*> pointLightAttributes;
		std::vector<Entity*> entities;

		std::vector<DirectionalLightObject*> directionalLights;
		std::vector<PointLightObject*> pointLights;

		nlohmann::ordered_json sceneJson;

	public:

		void Load(std::string path);

		void Save(std::string path);

		std::vector<Entity*>* pGetVectorOfEntities();

		Scene();

		std::vector<DataTypes::DirectionalLightAttributes_t*> GetVectorOfDirectionalLightAttributes();

		std::vector<DataTypes::PointLightAttributes_t*> GetVectorOfSpotlightAttributes();

		void Demo();

		void SetScrypts();

		void CleanScene();
	};

	namespace Globals{
		extern Scene* gScene;
	}
}

#endif