#pragma once
#include "Entities.h"
#include "Globals.h"
#include <vector>


namespace Lava{

	static void initBullet() {
		gBroadphase = new btDbvtBroadphase();
		gCollisionConfiguration = new btDefaultCollisionConfiguration();
		gDispatcher = new btCollisionDispatcher(gCollisionConfiguration);
		gSolver = new btSequentialImpulseConstraintSolver;
		gDynamicsWorld = new btDiscreteDynamicsWorld(gDispatcher, gBroadphase, gSolver, gCollisionConfiguration);
		gDynamicsWorld->setGravity(btVector3(0, -9.8, 0));
	}

	static void cleanBullet() {
		delete gSolver;
		delete gDispatcher;
		delete gCollisionConfiguration;
		delete gBroadphase;
		delete gDynamicsWorld;
	}

	class Scene {
		std::vector<SpotlightObject*> spotlights;
		std::vector<Entity*> entities;
	public:
		std::vector<Entity*> gVectorOfEntities(){
			return entities;
		}

		void InitScene(){
			initBullet();

			//  Объект, хранящий skybox
			CubemapObject* cubemapObject = new CubemapObject;

			std::vector<std::string> paths = {
			"CoreAssets/skybox/right.jpg",
			"CoreAssets/skybox/left.jpg",
			"CoreAssets/skybox/top.jpg",
			"CoreAssets/skybox/bottom.jpg",
			"CoreAssets/skybox/front.jpg",
			"CoreAssets/skybox/back.jpg"
			};

			cubemapObject->Init(
				paths
			);

			entities.push_back(cubemapObject);

			GameObject* gameObject = new GameObject;
			gameObject->GetMesh()->CreateMesh("CoreAssets/chair.obj");
			gameObject->SetID((int)entities.size());
			gameObject->SetName("chair");
			gameObject->GetMesh()->SetBaseColorTexture("CoreAssets/chair.png");
			gameObject->Transform.Scale(glm::vec3(0.2f, 0.2f, 0.2f));
			gameObject->Transform.Translate(glm::vec3(+8.0f, 10.0f, 5.0f));
			gameObject->GetRigidbody()->CreateRigidBody(
				LAVA_RIGIDBODY_SHAPE_TYPE_CUBE,
				1.1f,
				0.0f,
				2.0f,
				gDynamicsWorld,
				gameObject->GetID()
			);
			gameObject->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(gameObject);

			GameObject* gameObject1 = new GameObject;
			gameObject1->GetMesh()->CreateMesh("CoreAssets/table.obj");
			gameObject1->SetID((int)entities.size());
			gameObject1->SetName("Table");
			gameObject1->GetMesh()->SetBaseColorTexture("CoreAssets/table.jpg");
			gameObject1->Transform.Translate(glm::vec3(6.0f, 0.2f, 5.0f));
			gameObject1->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(gameObject1);

			GameObject* box = new GameObject;
			box->GetMesh()->CreateMesh("CoreAssets/AtlasCube.obj");
			box->Transform.Translate(glm::vec3(-15.0f, 5.0f, 0.0f));
			box->Transform.Rotate(glm::vec3(0.1f, 90.1f, 15.1f));
			box->GetMesh()->SetBaseColorTexture("CoreAssets/AtlasCube.png");

			box->SetID((int)entities.size());
			box->SetName("box1");
			box->GetRigidbody()->CreateRigidBody(
				LAVA_RIGIDBODY_SHAPE_TYPE_CUBE,
				1.1f,
				-1.0f,
				2.0f,
				gDynamicsWorld,
				box->GetID()
			);
			box->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));
			box->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(box);

			GameObject* box2 = new GameObject;
			box2->GetMesh()->CreateMesh("CoreAssets/AtlasCube.obj");
			box2->SetID((int)entities.size());
			box2->SetName("box2");
			box2->GetMesh()->SetBaseColorTexture("CoreAssets/AtlasCube.png");
			box2->Transform.Translate(glm::vec3(+-10.0f, 4.0f, 1.0f));
			box2->Transform.Scale(glm::vec3(3.5f, 1.5f, 1.5f));
			box2->Transform.Rotate(glm::vec3(0.1f, 90.1f, 15.1f));
			box2->GetRigidbody()->CreateRigidBody(
				LAVA_RIGIDBODY_SHAPE_TYPE_CUBE,
				1.1f,
				-1.0f,
				2.0f,
				gDynamicsWorld,
				box2->GetID()
			);
			box2->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(box2);

			GameObject* box3 = new GameObject;
			box3->GetMesh()->CreateMesh("CoreAssets/AtlasCube.obj");
			box3->SetID((int)entities.size());
			box3->SetName("box3");
			box3->GetMesh()->SetBaseColorTexture("CoreAssets/AtlasCube.png");
			box3->Transform.Translate(glm::vec3(0.0f, 5.0f, 0.0f));
			box3->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));
			box3->Transform.Rotate(glm::vec3(20.1f, 90.1f, 15.1f));
			box3->GetRigidbody()->CreateRigidBody(
				LAVA_RIGIDBODY_SHAPE_TYPE_CUBE,
				1.1f,
				-1.0f,
				2.0f,
				gDynamicsWorld,
				box3->GetID()
			);
			box3->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(box3);

			GameObject* gameObject2 = new GameObject;
			gameObject2->GetMesh()->CreateMesh("CoreAssets/plane.obj");
			gameObject2->SetID((int)entities.size());
			gameObject2->SetName("Arena");
			gameObject2->GetMesh()->SetBaseColorTexture("CoreAssets/ceramic2.jpg");
			gameObject2->GetRigidbody()->CreateRigidBody(
				LAVA_RIGIDBODY_SHAPE_TYPE_PLANE,
				0.0f,
				0.5f,
				0.5f,
				gDynamicsWorld,
				gameObject2->GetID()
			);
			gameObject2->Transform.Scale(glm::vec3(4.0f, 4.0f, 4.0f));
			gameObject2->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(gameObject2);


		
		}
	
		void CleanScene(){
			for (size_t i = 0; i < entities.size(); i++) {
				entities[i]->Destroy();
				delete entities[i];
			}
			cleanBullet();
		}
	}gScene;

}

