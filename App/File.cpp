#include <../EngineProject/src/Headers/Application.h>
#include "FloorScript.h"
extern Application app;
void Engine::Scene::Demo(){
	InitBullet();

	std::vector<std::string> paths = {
	"CoreAssets/skybox/right.jpg",
	"CoreAssets/skybox/left.jpg",
	"CoreAssets/skybox/top.jpg",
	"CoreAssets/skybox/bottom.jpg",
	"CoreAssets/skybox/front.jpg",
	"CoreAssets/skybox/back.jpg"
	};

	//  Объект, хранящий skybox
	CubemapObject* cubemapObject = new CubemapObject(paths);
	entities.push_back(cubemapObject);

	GameObject* gameObject = new GameObject;
	gameObject->pMesh = new Mesh;
	gameObject->pRigidBody = new RigidBody;

	gameObject->pMesh->CreateMesh("CoreAssets/chair.obj");
	gameObject->SetID((int)entities.size());
	gameObject->SetName("chair");
	gameObject->pMesh->SetBaseColorTexture("CoreAssets/chair.png");
	gameObject->Transform.Scale(glm::vec3(0.2f, 0.2f, 0.2f));
	gameObject->Transform.Translate(glm::vec3(+8.0f, 10.0f, 5.0f));
	gameObject->pRigidBody->CreateRigidBody(
		RIGIDBODY_SHAPE_TYPE_CUBE,
		1.1f,
		0.0f,
		2.0f,
		Globals::gDynamicsWorld,
		gameObject->GetID()
	);
	gameObject->ApplyEntityTransformToRigidbody();
	entities.push_back(gameObject);


	GameObject* gameObject1 = new GameObject;
	gameObject1->pMesh = new Mesh;
	gameObject1->pMesh->CreateMesh("CoreAssets/table.obj");
	gameObject1->SetID((int)entities.size());
	gameObject1->SetName("Table");
	gameObject1->pMesh->SetBaseColorTexture("CoreAssets/table.jpg");
	gameObject1->Transform.Translate(glm::vec3(6.0f, 0.2f, 5.0f));
	gameObject1->ApplyEntityTransformToRigidbody();
	entities.push_back(gameObject1);

	

	for (size_t i = 1; i < 40; i++)
	{
		GameObject* box = new GameObject;
		box->pMesh = new Mesh;
		box->pRigidBody = new RigidBody;


		box->pMesh->CreateMesh("CoreAssets/AtlasCube.obj");
		box->Transform.Translate(glm::vec3(i*3, i*3, i*3));
		box->Transform.Rotate(glm::vec3(0.1f, 90.1f, 15.1f));
		box->pMesh->SetBaseColorTexture("CoreAssets/AtlasCube.png");

		box->SetID((int)entities.size());
		box->SetName("box1");
		box->pRigidBody->CreateRigidBody(
			RIGIDBODY_SHAPE_TYPE_CUBE,
			1.1f,
			3.0f,
			2.0f,
			Globals::gDynamicsWorld,
			box->GetID()
		);
		box->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));
		box->pRigidBody->SetRigidbodyScale(glm::vec3(1.5f, 1.5f, 1.5f));
		box->ApplyEntityTransformToRigidbody();
		entities.push_back(box);
	}
	

	/*GameObject* box2 = new GameObject;
	box2->pMesh = new Mesh;
	box2->pRigidBody = new RigidBody;

	box2->pMesh->CreateMesh("CoreAssets/AtlasCube.obj");
	box2->SetID((int)entities.size());
	box2->SetName("box2");
	box2->pMesh->SetBaseColorTexture("CoreAssets/AtlasCube.png");
	box2->Transform.Translate(glm::vec3(+-10.0f, 4.0f, 1.0f));
	box2->Transform.Scale(glm::vec3(3.5f, 1.5f, 1.5f));
	box2->Transform.Rotate(glm::vec3(0.1f, 90.1f, 15.1f));
	box2->pRigidBody->SetRigidbodyScale(glm::vec3(3.5, 1.5, 1.5));

	box2->pRigidBody->CreateRigidBody(
		RIGIDBODY_SHAPE_TYPE_CUBE,
		1.1f,
		-1.0f,
		2.0f,
		Globals::gDynamicsWorld,
		box2->GetID()
	);
	box2->ApplyEntityTransformToRigidbody();
	entities.push_back(box2);

	GameObject* box3 = new GameObject;
	box3->pMesh = new Mesh;
	box3->pRigidBody = new RigidBody;

	box3->pMesh->CreateMesh("CoreAssets/AtlasCube.obj");
	box3->SetID((int)entities.size());
	box3->SetName("box3");
	box3->pMesh->SetBaseColorTexture("CoreAssets/AtlasCube.png");
	box3->Transform.Translate(glm::vec3(0.0f, 5.0f, 0.0f));
	box3->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));
	box3->Transform.Rotate(glm::vec3(20.1f, 90.1f, 15.1f));
	box3->pRigidBody->CreateRigidBody(
		RIGIDBODY_SHAPE_TYPE_CUBE,
		1.1f,
		-1.0f,
		2.0f,
		Globals::gDynamicsWorld,
		box3->GetID()
	);
	box3->ApplyEntityTransformToRigidbody();
	entities.push_back(box3);*/

	GameObject* gameObject2 = new Floor;
	gameObject2->pMesh = new Mesh;
	gameObject2->pRigidBody = new RigidBody;

	gameObject2->pMesh->CreateMesh("CoreAssets/plane.obj");
	gameObject2->SetID((int)entities.size());
	gameObject2->SetName("Arena");
	gameObject2->pMesh->SetBaseColorTexture("CoreAssets/ceramic2.jpg");
	gameObject2->pRigidBody->CreateRigidBody(
		RIGIDBODY_SHAPE_TYPE_PLANE,
		0.0f,
		0.5f,
		0.5f,
		Globals::gDynamicsWorld,
		gameObject2->GetID()
	);
	gameObject2->Transform.Scale(glm::vec3(70.0f, 70.0f, 70.0f));
	gameObject2->pRigidBody->SetRigidbodyScale(gameObject2->Transform.GetScaleValue());
	gameObject2->ApplyEntityTransformToRigidbody();
	entities.push_back(gameObject2);


	//GameObject* gameObject3 = new GameObject;
	//gameObject3->pMesh = new Mesh;

	//gameObject3->pMesh->CreateMesh("CoreAssets/City.obj");
	//gameObject3->SetID((int)entities.size());
	//gameObject3->SetName("City");
	//
	//entities.push_back(gameObject3);

	//GameObject* gameObject5 = new GameObject;
	//gameObject5->pMesh = new Mesh;

	//gameObject5->pMesh->CreateMesh("CoreAssets/guy.obj");
	//gameObject5->SetID((int)entities.size());
	//gameObject5->SetName("Guy");
	//gameObject5->pMesh->SetBaseColorTexture("CoreAssets/guy.jpg");
	//gameObject5->Transform.Scale(glm::vec3(0.3f, 0.3f, 0.3f));
	//gameObject5->ApplyEntityTransformToRigidbodyAndMesh();
	//entities.push_back(gameObject5);

	SpotlightObject* spotlight = new SpotlightObject;
	spotlight->Transform.Translate(glm::vec3(-50.0f, 3.0f, 2.0f));
	spotlight->SetID((int)entities.size());
	entities.push_back(spotlight);

	SpotlightObject* spotlight2 = new SpotlightObject;
	spotlight2->Transform.Translate(glm::vec3(25.0f, 3.0f, 13.0f));
	spotlight2->SetID((int)entities.size());
	entities.push_back(spotlight2);

	SpotlightObject* spotlight3 = new SpotlightObject;
	spotlight3->Transform.Translate(glm::vec3(50.0f, 3.0f, 13.0f));
	spotlight3->SetID((int)entities.size());
	entities.push_back(spotlight3);

	SpotlightObject* spotlight4 = new SpotlightObject;
	spotlight4->Transform.Translate(glm::vec3(70.0f, 3.0f, 13.0f));
	spotlight4->SetID((int)entities.size());
	entities.push_back(spotlight4);

	//GameObject* gameObject3 = new GameObject;
	//gameObject3->pMesh = new Mesh;
	//gameObject3->SetName("Moon");
	//gameObject3->SetID(entities.size());
	//gameObject3->pMesh->CreateMesh("CoreAssets/moon.obj");
	//gameObject3->pMesh->SetBaseColorTexture("CoreAssets/moon.jpg");
	//entities.push_back(gameObject3);


	spotlightAttributes.push_back(spotlight->pGetSpotlightUniformData());
	spotlightAttributes.push_back(spotlight2->pGetSpotlightUniformData());
	spotlightAttributes.push_back(spotlight3->pGetSpotlightUniformData());
	spotlightAttributes.push_back(spotlight4->pGetSpotlightUniformData());

}