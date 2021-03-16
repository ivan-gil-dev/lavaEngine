#ifndef scene_h
#define scene_h


#include "Entities.h"
#include "Globals.h"
#include "DataTypes.h"
#include <vector>


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
		std::vector<SpotlightObject*> spotlights;
		std::vector<DataTypes::SpotlightAttributes_t*> spotlightAttributes;
		std::vector<Entity*> entities;
	public:

		std::vector<Entity*> GetVectorOfEntities(){
			return entities;
		}

		Scene(){
			
		}

		std::vector<DataTypes::SpotlightAttributes_t*> GetVectorOfSpotlightAttributes(){
			return spotlightAttributes;
		}

		void Demo();

		void CleanScene(){

			for (size_t i = 0; i < entities.size(); i++) {
				entities[i]->Destroy();
				delete entities[i];
			}
		
			CleanBullet();
		}
	};

	namespace Globals{
		extern Scene* gScene;
	}
}

#endif