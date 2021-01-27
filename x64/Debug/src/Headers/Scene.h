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
		std::vector<Entity*> entities;
	public:
		std::vector<Entity*> GetVectorOfEntities(){
			return entities;
		}

		void InitScene(){
			initBullet();

			//  Объект, хранящий skybox
			CubemapObject* cubemapObject = new CubemapObject;

			std::vector<std::string> paths = {
			"assets/skybox/right.jpg",
			"assets/skybox/left.jpg",
			"assets/skybox/top.jpg",
			"assets/skybox/bottom.jpg",
			"assets/skybox/front.jpg",
			"assets/skybox/back.jpg"
			};

			cubemapObject->Init(
				paths
			);

			entities.push_back(cubemapObject);

			GameObject* gameObject = new GameObject;
			gameObject->GetMesh()->CreateMesh("assets/chair.obj");
			gameObject->SetId((int)entities.size());
			gameObject->SetName("chair");
			gameObject->GetMesh()->AddBaseColorTexture("assets/chair.png");
			gameObject->Transform.Scale(glm::vec3(0.2f, 0.2f, 0.2f));
			gameObject->Transform.SetStartTranslation(glm::vec3(+8.0f, 10.0f, 5.0f));
			gameObject->GetRigidBody()->CreateRigidBody(
				gameObject->GetMesh(),
				LAVA_RIGIDBODY_SHAPE_TYPE_CUBE,
				1.1f,
				0.0f,
				2.0f,
				gDynamicsWorld,
				gameObject->GetId()
			);
			gameObject->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(gameObject);

			GameObject* gameObject1 = new GameObject;
			gameObject1->GetMesh()->CreateMesh("assets/table.obj");
			gameObject1->SetId((int)entities.size());
			gameObject1->SetName("Table");
			gameObject1->GetMesh()->AddBaseColorTexture("assets/table.jpg");
			gameObject1->Transform.SetStartTranslation(glm::vec3(6.0f, 0.2f, 5.0f));
			gameObject1->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(gameObject1);

			GameObject* box = new GameObject;
			box->GetMesh()->CreateMesh("assets/cube.obj");
			box->Transform.SetStartTranslation(glm::vec3(-15.0f, 5.0f, 0.0f));
			box->Transform.SetStartRotation(glm::vec3(0.1f, 90.1f, 15.1f));
			box->GetMesh()->AddBaseColorTexture("assets/texture1.jpg");

			box->SetId((int)entities.size());
			box->SetName("box1");
			box->GetRigidBody()->CreateRigidBody(
				box->GetMesh(),
				LAVA_RIGIDBODY_SHAPE_TYPE_MESH,
				1.1f,
				-1.0f,
				2.0f,
				gDynamicsWorld,
				box->GetId()
			);
			box->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));
			box->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(box);

			GameObject* box2 = new GameObject;
			box2->GetMesh()->CreateMesh("assets/cube.obj");
			box2->SetId((int)entities.size());
			box2->SetName("box2");
			box2->GetMesh()->AddBaseColorTexture("assets/texture1.jpg");
			box2->Transform.SetStartTranslation(glm::vec3(+-10.0f, 4.0f, 1.0f));
			box2->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));
			box2->Transform.SetStartRotation(glm::vec3(0.1f, 90.1f, 15.1f));
			box2->GetRigidBody()->CreateRigidBody(
				box2->GetMesh(),
				LAVA_RIGIDBODY_SHAPE_TYPE_MESH,
				1.1f,
				-1.0f,
				2.0f,
				gDynamicsWorld,
				box2->GetId()
			);
			box2->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(box2);

			GameObject* box3 = new GameObject;
			box3->GetMesh()->CreateMesh("assets/cube.obj");
			box3->SetId((int)entities.size());
			box3->SetName("box3");
			box3->GetMesh()->AddBaseColorTexture("assets/texture1.jpg");
			box3->Transform.SetStartTranslation(glm::vec3(0.0f, 5.0f, 0.0f));
			box3->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));
			box3->Transform.SetStartRotation(glm::vec3(20.1f, 90.1f, 15.1f));
			box3->GetRigidBody()->CreateRigidBody(
				box3->GetMesh(),
				LAVA_RIGIDBODY_SHAPE_TYPE_MESH,
				1.1f,
				-1.0f,
				2.0f,
				gDynamicsWorld,
				box3->GetId()
			);
			box3->ApplyEntityTransformToRigidbodyAndMesh();
			entities.push_back(box3);

			GameObject* gameObject2 = new GameObject;
			gameObject2->GetMesh()->CreateMesh("assets/arena.obj");
			gameObject2->SetId((int)entities.size());
			gameObject2->SetName("Arena");
			gameObject2->GetMesh()->AddBaseColorTexture("assets/ceramic2.jpg");
			gameObject2->GetRigidBody()->CreateRigidBody(
				gameObject2->GetMesh(),
				LAVA_RIGIDBODY_SHAPE_TYPE_PLANE,
				0.0f,
				0.5f,
				0.5f,
				gDynamicsWorld,
				gameObject2->GetId()
			);
			gameObject2->Transform.Scale(glm::vec3(2.0f, 2.0f, 2.0f));
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

