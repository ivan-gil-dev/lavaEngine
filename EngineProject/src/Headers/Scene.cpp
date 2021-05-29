#include "Scene.h"
#include "Renderer/Renderer.h"
#include <mutex>
#include <chrono>
std::string Engine::Scene::GetScenePath()
{
    return scenePath;
}

void Engine::Scene::Load(const std::string& path)
{

    for (size_t i = 0; i < entities.size(); i++) {
        delete entities[i];
    }

    for (size_t i = 0; i < cameras.size(); i++)
    {
        delete cameras[i];
    }
    cameras.clear();

    activeCamera = nullptr;

    entities.clear();

    pointLightAttributes.clear();
    directionalLightAttributes.clear();
    
    sceneJson.clear();

    std::ifstream inputJson(path);

    if (!inputJson.is_open()) {
        spdlog::warn("Warning! Scene file is not found!");
        return;
    }

    scenePath = path;

    inputJson >> sceneJson;

    inputJson.close();

    //std::cout << sceneJson.dump() << std::endl;

    for (auto entityJson : sceneJson["Entities"]) {

        Entity* entity;
       

        if (entityJson["Type"] == "GameObject") {      
            entity = new GameObject;

            if (entityJson.count("Mesh") !=0 ){
                ((GameObject*)entity)->AddComponent<Mesh>();
                Mesh* mesh = ((GameObject*)entity)->pGetComponent<Mesh*>();
                mesh->CreateMesh(entityJson["Mesh"]["Path"]);
                DataTypes::Material_t mat;
                mat.shininess = entityJson["Mesh"].value("Shininess", 80.0f);
                mat.roughness = entityJson["Mesh"].value("Roughness", 1.0f);
                mat.metallic = entityJson["Mesh"].value("Metallic", 1.0f);
                mat.ao = entityJson["Mesh"].value("Occlusion", 1.0f);
                mesh->SetMaterial(mat);
            }

            if (entityJson.count("Rigidbody") != 0) {
                ((GameObject*)entity)->AddComponent<RigidBody>();
                RigidBody* rigidbody = ((GameObject*)entity)->pGetComponent<RigidBody*>();

                std::string shapeType = entityJson["Rigidbody"]["ShapeType"];

                if (shapeType == "Plane"){
                    rigidbody->CreateRigidBody(
                        RIGIDBODY_SHAPE_TYPE_PLANE,
                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                        int(entity));
                }

                if (shapeType == "Cube") {
                    rigidbody->CreateRigidBody(
                        RIGIDBODY_SHAPE_TYPE_CUBE,
                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                        int(entity));
                }
                
                if (shapeType == "Sphere") {
                    rigidbody->CreateRigidBody(
                        RIGIDBODY_SHAPE_TYPE_SPHERE,
                        Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld,
                        int(entity));
                }


                rigidbody->SetMass(entityJson["Rigidbody"]["Mass"]);
                rigidbody->SetFriction(entityJson["Rigidbody"]["Friction"]);
                rigidbody->SetRestitution(entityJson["Rigidbody"]["Restitution"]);
                 
                rigidbody->SetRigidbodyScale(glm::vec3(
                    entityJson["Rigidbody"]["Scale"]["X"],
                    entityJson["Rigidbody"]["Scale"]["Y"],
                    entityJson["Rigidbody"]["Scale"]["Z"]
                ));
            }

        }

        else if (entityJson["Type"] == "PointLight") {
           // std::cout << entityJson["Type"] << std::endl;
            entity = new PointLightObject;
            DataTypes::PointLightAttributes_t* attrib = ((PointLightObject*)entity)->pGetPointLightUniformData();

            attrib->constant = entityJson["Constant"];
            attrib->linear = entityJson["Linear"];
            attrib->quadrantic = entityJson["Quadrantic"];

            attrib->lightPosition = glm::vec3(
                entityJson.at("LightPosition").at("X"),
                entityJson["LightPosition"]["Y"],
                entityJson["LightPosition"]["Z"] 
            );
                
            attrib->lightColor = glm::vec3(
                entityJson["LightColor"]["R"],
                entityJson["LightColor"]["G"],
                entityJson["LightColor"]["B"]
            );

           
            attrib->ambient = entityJson.value("Ambient",1); 
            attrib->diffuse = entityJson.value("Diffuse",1);
            attrib->specular = entityJson.value("Specular",1);
            
            pointLightAttributes.push_back(attrib);
        }

        else if (entityJson["Type"] == "DirectionalLight") {
           // std::cout << entityJson["Type"] << std::endl;
            entity = new DirectionalLightObject;
            DataTypes::DirectionalLightAttributes_t* attrib = ((DirectionalLightObject*)entity)->pGetDirectionalLightUniformData();

            attrib->lightDirection.x = entityJson["Direction"]["X"];
            attrib->lightDirection.y = entityJson["Direction"]["Y"];
            attrib->lightDirection.z = entityJson["Direction"]["Z"];

            attrib->lightColor.r = entityJson["Color"]["R"];
            attrib->lightColor.g = entityJson["Color"]["G"];
            attrib->lightColor.b = entityJson["Color"]["B"];

            attrib->ambient = entityJson.value("Ambient", 1);
            attrib->diffuse = entityJson.value("Diffuse", 0.5);
            attrib->specular = entityJson.value("Specular", 1);

            directionalLightAttributes.push_back(attrib);
        }

        else if (entityJson["Type"] == "Cubemap") {
           // std::cout << entityJson["Type"] << std::endl;
            std::vector<std::string> cubemapPaths;

            for (auto cubemapPath : entityJson["CubemapPaths"]){
                cubemapPaths.push_back(cubemapPath);
            }

            entity = new CubemapObject(cubemapPaths);
        }
        else {
            spdlog::warn("Warning! Wrong entity type in the file!");
            break;
        }

        entity->SetID((int)entity);
        entity->SetName(entityJson["Name"]);

        glm::vec3 Position;
        Position.x = entityJson["Transform"]["Position"]["X"];
        Position.y = entityJson["Transform"]["Position"]["Y"];
        Position.z = entityJson["Transform"]["Position"]["Z"];

        glm::vec3 Rotation;
        Rotation.x = entityJson["Transform"]["Rotation"]["X"];
        Rotation.y = entityJson["Transform"]["Rotation"]["Y"];
        Rotation.z = entityJson["Transform"]["Rotation"]["Z"];

        glm::vec3 Scale;
        Scale.x = entityJson["Transform"]["Scale"]["X"];
        Scale.y = entityJson["Transform"]["Scale"]["Y"];
        Scale.z = entityJson["Transform"]["Scale"]["Z"];

        entity->Transform.Translate(Position);
        entity->Transform.Rotate(Rotation);
        entity->Transform.Scale(Scale);

        if (entityJson["Type"] == "GameObject") {
            ((GameObject*)entity)->ApplyEntityTransformToRigidbody();
        }

        entities.push_back(entity);
    }

}

void Engine::Scene::New()
{

    for (size_t i = 0; i < cameras.size(); i++)
    {
        delete cameras[i];
    }
    cameras.clear();
    activeCamera = nullptr;


    for (size_t i = 0; i < entities.size(); i++) {
        delete entities[i];
    }

    entities.clear();

    pointLightAttributes.clear();
    directionalLightAttributes.clear();

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

    GameObject* gameObject2 = new GameObject;
    gameObject2->AddComponent<Mesh>();
   
    Mesh* mesh;
    mesh = gameObject2->pGetComponent<Mesh*>();

    mesh->CreateMesh("CoreAssets/plane.obj");
   

    gameObject2->SetID((int)gameObject2);
    gameObject2->SetName("Plane");

    gameObject2->Transform.Scale(glm::vec3(70.0f, 70.0f, 70.0f));
    entities.push_back(gameObject2);


    DirectionalLightObject* dlight = new DirectionalLightObject;
    dlight->pGetDirectionalLightUniformData()->lightDirection = glm::vec3(1, -1, 1);
    dlight->pGetDirectionalLightUniformData()->lightColor = glm::vec3(1, 1, 1);

    dlight->SetID((int)dlight);
    entities.push_back(dlight);
    directionalLightAttributes.push_back(dlight->pGetDirectionalLightUniformData());

}

void Engine::Scene::Save()
{
    SaveAs(scenePath);
}

void Engine::Scene::SaveAs(const std::string& path)
{
    scenePath = path;
    std::ofstream outputJson(path);
    sceneJson.clear();
    for (int i = 0; i < entities.size(); i++) {
        sceneJson["Entities"][i]["Name"] = entities[i]->GetName();

        EntityType entityType = EntityType(entities[i]->GetEntityType());
        if (entityType == ENTITY_TYPE_GAME_OBJECT) {
            sceneJson["Entities"][i]["Type"] = "GameObject";
            Mesh* mesh = ((GameObject*)entities[i])->pGetComponent<Mesh*>();
            if (mesh != nullptr) {
                sceneJson["Entities"][i]["Mesh"]["Path"] = mesh->pGetMeshPath();
                DataTypes::Material_t mat = mesh->GetMaterial();
                sceneJson["Entities"][i]["Mesh"]["Shininess"] = mat.shininess;
                sceneJson["Entities"][i]["Mesh"]["Roughness"] = mat.roughness;
                sceneJson["Entities"][i]["Mesh"]["Metallic"]  = mat.metallic;
                sceneJson["Entities"][i]["Mesh"]["Occlusion"] = mat.ao;
            }
            RigidBody* rigidBody = ((GameObject*)entities[i])->pGetComponent<RigidBody*>();
            if (rigidBody != nullptr) {
                sceneJson["Entities"][i]["Rigidbody"]["Mass"] = rigidBody->GetBulletRigidBody()->getMass();
                sceneJson["Entities"][i]["Rigidbody"]["Restitution"] = rigidBody->GetBulletRigidBody()->getRestitution();
                sceneJson["Entities"][i]["Rigidbody"]["Friction"] = rigidBody->GetBulletRigidBody()->getFriction();
                sceneJson["Entities"][i]["Rigidbody"]["Scale"]["X"] = rigidBody->GetRigidbodyScale().x;
                sceneJson["Entities"][i]["Rigidbody"]["Scale"]["Y"] = rigidBody->GetRigidbodyScale().y;
                sceneJson["Entities"][i]["Rigidbody"]["Scale"]["Z"] = rigidBody->GetRigidbodyScale().z;

                if (rigidBody->GetShapeType() == RIGIDBODY_SHAPE_TYPE_CUBE) {
                    sceneJson["Entities"][i]["Rigidbody"]["ShapeType"] = "Cube";
                }

                if (rigidBody->GetShapeType() == RIGIDBODY_SHAPE_TYPE_PLANE) {
                    sceneJson["Entities"][i]["Rigidbody"]["ShapeType"] = "Plane";
                }

                if (rigidBody->GetShapeType() == RIGIDBODY_SHAPE_TYPE_SPHERE) {
                    sceneJson["Entities"][i]["Rigidbody"]["ShapeType"] = "Sphere";
                }
            }
        }
        if (entityType == ENTITY_TYPE_POINTLIGHT_OBJECT) {
            sceneJson["Entities"][i]["Type"] = "PointLight";
            DataTypes::PointLightAttributes_t attributes = *((PointLightObject*)entities[i])->pGetPointLightUniformData();

            sceneJson["Entities"][i]["LightPosition"]["X"] = attributes.lightPosition.x;
            sceneJson["Entities"][i]["LightPosition"]["Y"] = attributes.lightPosition.y;
            sceneJson["Entities"][i]["LightPosition"]["Z"] = attributes.lightPosition.z;

            sceneJson["Entities"][i]["LightColor"]["R"] = attributes.lightColor.x;
            sceneJson["Entities"][i]["LightColor"]["G"] = attributes.lightColor.y;
            sceneJson["Entities"][i]["LightColor"]["B"] = attributes.lightColor.z;

            sceneJson["Entities"][i]["Constant"] = attributes.constant;
            sceneJson["Entities"][i]["Linear"] = attributes.linear;
            sceneJson["Entities"][i]["Quadrantic"] = attributes.quadrantic;


            sceneJson["Entities"][i]["Ambient"] = attributes.ambient;
            sceneJson["Entities"][i]["Diffuse"] = attributes.diffuse;
            sceneJson["Entities"][i]["Specular"] = attributes.specular;
        }
        if (entityType == ENTITY_TYPE_DIRECTIONAL_LIGHT_OBJECT) {
            sceneJson["Entities"][i]["Type"] = "DirectionalLight";
            DataTypes::DirectionalLightAttributes_t attributes = *((DirectionalLightObject*)entities[i])->pGetDirectionalLightUniformData();

            sceneJson["Entities"][i]["Direction"]["X"] = attributes.lightDirection.x;
            sceneJson["Entities"][i]["Direction"]["Y"] = attributes.lightDirection.y;
            sceneJson["Entities"][i]["Direction"]["Z"] = attributes.lightDirection.z;

            sceneJson["Entities"][i]["Color"]["R"] = attributes.lightColor.r;
            sceneJson["Entities"][i]["Color"]["G"] = attributes.lightColor.g;
            sceneJson["Entities"][i]["Color"]["B"] = attributes.lightColor.b;
           
            sceneJson["Entities"][i]["Ambient"] = attributes.ambient;
            sceneJson["Entities"][i]["Diffuse"] = attributes.diffuse;
            sceneJson["Entities"][i]["Specular"] = attributes.specular;

        }
        if (entityType == ENTITY_TYPE_CUBEMAP_OBJECT) {
            sceneJson["Entities"][i]["Type"] = "Cubemap";
            for (size_t j = 0; j < ((CubemapObject*)entities[i])->GetCubemapPaths().size(); j++) {
                sceneJson["Entities"][i]["CubemapPaths"][j] = ((CubemapObject*)entities[i])->GetCubemapPaths()[j];
            }
        }

        sceneJson["Entities"][i]["Transform"]["Position"]["X"] = entities[i]->Transform.Position.x;
        sceneJson["Entities"][i]["Transform"]["Position"]["Y"] = entities[i]->Transform.Position.y;
        sceneJson["Entities"][i]["Transform"]["Position"]["Z"] = entities[i]->Transform.Position.z;
        sceneJson["Entities"][i]["Transform"]["Scale"]["X"] = entities[i]->Transform.ScaleValue.x;
        sceneJson["Entities"][i]["Transform"]["Scale"]["Y"] = entities[i]->Transform.ScaleValue.y;
        sceneJson["Entities"][i]["Transform"]["Scale"]["Z"] = entities[i]->Transform.ScaleValue.z;
        sceneJson["Entities"][i]["Transform"]["Rotation"]["X"] = entities[i]->Transform.EulerAngles.x;
        sceneJson["Entities"][i]["Transform"]["Rotation"]["Y"] = entities[i]->Transform.EulerAngles.y;
        sceneJson["Entities"][i]["Transform"]["Rotation"]["Z"] = entities[i]->Transform.EulerAngles.z;
    }

    //std::cout << sceneJson.dump(1) << std::endl;
    outputJson << sceneJson.dump(1);
    outputJson.close();
}

void Engine::Scene::New_FromThread(std::atomic<bool>& ready)
{
    std::mutex m;
    m.lock();
    ready = false;
    New();
    ready = true;
    m.unlock();
}

void Engine::Scene::Save_FromThread(std::atomic<bool>& ready)
{
    std::mutex m;
    m.lock();
    ready = false;
    Save();
    ready = true;
    m.unlock();
}

void Engine::Scene::SaveAs_FromThread(std::string path, std::atomic<bool>& ready)
{
    std::mutex m;
    m.lock();
    ready = false;
    SaveAs(path);
    ready = true;
    m.unlock();
}

void Engine::Scene::Load_FromThread(std::string path, std::atomic<bool>& ready)
{
    std::mutex m;
    m.lock();

    ready = false;
    Load(path);
    ready = true;
    m.unlock();
}

std::vector<Engine::Entity*>* Engine::Scene::pGetVectorOfEntities()
{
    return &entities;
}

Engine::Scene::Scene()
{
    Engine::Globals::bulletPhysicsGlobalObjects.InitBullet();
    scenePath = "";
    #ifdef DemoFromFile
        Load("demo.json");
    #endif
    entities.resize(0);
    directionalLightAttributes.resize(0);
    pointLightAttributes.resize(0);
}

std::vector<Engine::DataTypes::DirectionalLightAttributes_t*>* Engine::Scene::pGetVectorOfDirectionalLightAttributes()
{
    return &directionalLightAttributes;
}

std::vector<Engine::DataTypes::PointLightAttributes_t*>* Engine::Scene::pGetVectorOfSpotlightAttributes()
{
    return &pointLightAttributes;
}

std::vector<Engine::Camera*>* Engine::Scene::pGetVectorOfCameras()
{
    return &cameras;
}

void Engine::Scene::SetActiveCameraFromIndex(int id)
{
    if (id<cameras.size())
    {
        activeCamera = cameras[id];
    }
}

Engine::Camera* Engine::Scene::pGetActiveCamera()
{
    return activeCamera;
}

Engine::Camera Engine::Scene::GetActiveCamera()
{
    return *activeCamera;
}

void Engine::Scene::UpdateActiveCamera()
{
    activeCamera->Update();
}

void Engine::Scene::CleanScene()
{
    //Save();

    for (size_t i = 0; i < cameras.size(); i++)
    {
        delete cameras[i];
    }

    for (size_t i = 0; i < entities.size(); i++) {
        delete entities[i];
    }

    Engine::Globals::bulletPhysicsGlobalObjects.CleanBullet();
}
