#include <../EngineProject/src/Headers/Application.h>
#include "FloorScript.h"

extern Application app;


void Engine::Scene::Demo(){
	//
	Mesh* mesh;
	RigidBody* rigidBody;

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

    for (size_t i = 1; i < 40; i++) {
        GameObject* box = new GameObject;
        box->AddComponent<Mesh>();
        box->AddComponent<RigidBody>();

        mesh = box->pGetComponent<Mesh*>();
        rigidBody = box->pGetComponent<RigidBody*>();

        mesh->CreateMesh("CoreAssets/AtlasCube.obj");

        rigidBody->CreateRigidBody(
            RIGIDBODY_SHAPE_TYPE_CUBE,
            Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
            (int)box
        );

        rigidBody->SetRestitution(2.0f);
        rigidBody->SetRigidbodyScale(glm::vec3(1.5f, 1.5f, 1.5f));

        box->Transform.Translate(glm::vec3(i * 3, i * 3, i * 3));
        box->Transform.Rotate(glm::vec3(0.1f, 90.1f, 15.1f));

        box->SetID((int)box);
        box->SetName("box1");

        box->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));

        box->ApplyEntityTransformToRigidbody();
        entities.push_back(box);
    }

    GameObject* gameObject2 = new Floor;
    gameObject2->AddComponent<Mesh>();
    gameObject2->AddComponent<RigidBody>();

    mesh = gameObject2->pGetComponent<Mesh*>();
    rigidBody = gameObject2->pGetComponent<RigidBody*>();

    mesh->CreateMesh("CoreAssets/ceramic.obj");
    rigidBody->CreateRigidBody(RIGIDBODY_SHAPE_TYPE_PLANE,
        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
        (int)gameObject2
    );

    gameObject2->SetID((int)gameObject2);
    gameObject2->SetName("Arena");

    gameObject2->Transform.Scale(glm::vec3(70.0f,70.0f, 70.0f));

    rigidBody->SetMass(0.0f);
    rigidBody->SetRigidbodyScale(gameObject2->Transform.GetScaleValue());


    gameObject2->ApplyEntityTransformToRigidbody();
    entities.push_back(gameObject2);

    GameObject* obj = new GameObject;
    obj->SetID((int)obj);
    obj->SetName("Test");
    entities.push_back(obj);

    DirectionalLightObject* dlight = new DirectionalLightObject;
    dlight->pGetDirectionalLightUniformData()->lightDirection = glm::vec3(1, -1, 1);
    dlight->pGetDirectionalLightUniformData()->lightColor = glm::vec3(1, 1, 1);

    dlight->SetID((int)dlight);
    entities.push_back(dlight);
    directionalLightAttributes.push_back(dlight->pGetDirectionalLightUniformData());


    DirectionalLightObject* dlight2 = new DirectionalLightObject;
    dlight2->pGetDirectionalLightUniformData()->lightDirection = glm::vec3(0.25, -1, 0.25);
    dlight2->pGetDirectionalLightUniformData()->lightColor = glm::vec3(0.2, 0.2, 0.2);

    dlight2->SetID((int)dlight2);
    entities.push_back(dlight2);


   
	directionalLightAttributes.push_back(dlight2->pGetDirectionalLightUniformData());

    PointLightObject* pointLight = new PointLightObject;
    pointLight->Transform.Translate(glm::vec3(-50.0f, 3.0f, 2.0f));
    pointLight->SetID((int)pointLight);
    entities.push_back(pointLight);

    PointLightObject* pointLight2 = new PointLightObject;
    pointLight2->Transform.Translate(glm::vec3(25.0f, 3.0f, 13.0f));
    pointLight2->SetID((int)pointLight2);
    entities.push_back(pointLight2);

    PointLightObject* pointLight3 = new PointLightObject;
    pointLight3->Transform.Translate(glm::vec3(50.0f, 3.0f, 13.0f));
    pointLight3->SetID((int)pointLight3);
    entities.push_back(pointLight3);

    PointLightObject* pointLight4 = new PointLightObject;
    pointLight4->Transform.Translate(glm::vec3(70.0f, 3.0f, 13.0f));
    pointLight4->SetID((int)pointLight4);
    entities.push_back(pointLight4);

	pointLightAttributes.push_back(pointLight->pGetPointLightUniformData());
	pointLightAttributes.push_back(pointLight2->pGetPointLightUniformData());
	pointLightAttributes.push_back(pointLight3->pGetPointLightUniformData());
	pointLightAttributes.push_back(pointLight4->pGetPointLightUniformData());
}