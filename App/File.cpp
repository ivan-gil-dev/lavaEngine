#include <../EngineProject/src/Headers/Application.h>
#include "FloorScript.h"

extern Application app;


class PlayerCamera : public Engine::Camera {
    glm::vec3 direction;
    int counter = 0;
public:
    PlayerCamera() {
        direction = glm::vec3(1, -1, 1);
    }
    void MouseUpdate()
    {
        using namespace Engine;
        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_ALT)) {
            Globals::showCursorEventHandler.HideCursor();

           

            


            double xpos = Globals::cursorPosition.GetCursorPos().x,
                   ypos = Globals::cursorPosition.GetCursorPos().y;


          

             
            


            if (CursorFirstMouse) {
                CursorLastX = xpos;
                CursorLastY = ypos;
                CursorFirstMouse = false;
            }

            double xoffset = xpos - CursorLastX;
            double yoffset = CursorLastY - ypos;

            CursorLastX = xpos;
            CursorLastY = ypos;

            float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

           
            
           
      
          



            Yaw = std::fmod((Yaw + xoffset), (GLfloat)360.0f);
            Pitch += yoffset;

            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;

            {
                
                direction.z = glm::sin(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
                direction.y = glm::sin(glm::radians((float)Pitch));
                direction.x = glm::cos(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
     
                //CameraFront = glm::normalize(direction);
            }
            

            
        }
        else {
            ClipCursor(NULL);
            Globals::showCursorEventHandler.ShowCursor();
            CursorFirstMouse = true;
        }
    }

    void   Update() override {
        MouseUpdate();
        glm::vec3 playerPos = ref->Transform.GetPosition() ;
        CameraPos = playerPos + glm::vec3(20.0f, 20.0f, 20.f)*(-direction);
        CameraFront = glm::normalize(playerPos - CameraPos);
    }
};


class Player : public Engine::GameObject {
    float Speed = 10.1f;
public:
    void Update() override{
        using namespace Engine;
        glm::vec3 CameraFront = ((Camera*)ref)->GetCameraFront();
        btVector3 dir = btVector3(CameraFront.x, 0, CameraFront.z);

        if (Globals::keyPressedEventHandler.IsKeyPressed(Engine::KEY_W))
        {
            dir = btVector3(CameraFront.x * Speed * Globals::DeltaTime, 0, CameraFront.z * Speed * Globals::DeltaTime);
            pRigidBody->GetBulletRigidBody()->applyCentralImpulse(dir);
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(Engine::KEY_S))
        {
            dir = btVector3(CameraFront.x * Speed * Globals::DeltaTime, 0, CameraFront.z * Speed * Globals::DeltaTime);
            pRigidBody->GetBulletRigidBody()->applyCentralImpulse(-dir);
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(Engine::KEY_A))
        {
            glm::vec3 cross = glm::cross(CameraFront, glm::vec3(0, 1, 0));
            dir = btVector3(cross.x * Speed * Globals::DeltaTime, 0, cross.z * Speed * Globals::DeltaTime);
            pRigidBody->GetBulletRigidBody()->applyCentralImpulse(-dir);
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(Engine::KEY_D))
        {
            glm::vec3 cross = glm::cross(CameraFront, glm::vec3(0, 1, 0));
            dir = btVector3(cross.x * Speed * Globals::DeltaTime, 0, cross.z * Speed * Globals::DeltaTime);
            pRigidBody->GetBulletRigidBody()->applyCentralImpulse(dir);
        }
    }
};



extern "C" {
    __declspec(dllexport) void DemoExe(
        Engine::Scene *scene,
        btDynamicsWorld *dynamicsWorld
    )
    {
        using namespace Engine;


        std::vector<Entity*> *entities = scene->pGetVectorOfEntities();
        std::vector<DataTypes::DirectionalLightAttributes_t*>* directionalLightAttributes = scene->pGetVectorOfDirectionalLightAttributes();
        std::vector<DataTypes::PointLightAttributes_t*>* pointLightAttributes = scene->pGetVectorOfSpotlightAttributes();
        std::vector<Engine::Camera*>* cameras = scene->pGetVectorOfCameras();

        Camera* cam = new PlayerCamera;
        cam->SetCameraFront(glm::vec3(0.f, -1.f, -1.f));
        cam->SetCameraPos(glm::vec3(5.0f, 50.0f, 50.f));
        cameras->push_back(cam);
        scene->SetActiveCameraFromIndex(0);


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
        entities->push_back(cubemapObject);

        for (size_t i = 1; i < 40; i++) {
            if (i == 1)
            {
                GameObject* Sphere = new Player;
                Sphere->AddComponent<Mesh>();
                Sphere->AddComponent<RigidBody>();

                mesh = Sphere->pGetComponent<Mesh*>();
                rigidBody = Sphere->pGetComponent<RigidBody*>();

                mesh->CreateMesh("CoreAssets/sphere2.obj");

                rigidBody->CreateRigidBody(
                    RIGIDBODY_SHAPE_TYPE_SPHERE,
                    dynamicsWorld,
                    reinterpret_cast<int>(Sphere)
                );

                rigidBody->SetRestitution(2.0f);
                rigidBody->SetRigidbodyScale(glm::vec3(3.f, 3.f, 3.f));

                Sphere->Transform.Translate(glm::vec3(i * 3, i * 3, i * 3));
                Sphere->Transform.Rotate(glm::vec3(0.1f, 90.1f, 15.1f));

                Sphere->SetID((int)Sphere);
                Sphere->SetName("box1");

                Sphere->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));

                Sphere->ApplyEntityTransformToRigidbody();
                entities->push_back(Sphere);

                cam->SetRef(Sphere);
                Sphere->SetRef(cam);

            }
            else {
                GameObject* box = new GameObject;
                box->AddComponent<Mesh>();
                box->AddComponent<RigidBody>();

                mesh = box->pGetComponent<Mesh*>();
                rigidBody = box->pGetComponent<RigidBody*>();

                mesh->CreateMesh("CoreAssets/AtlasCube.obj");

                rigidBody->CreateRigidBody(
                    RIGIDBODY_SHAPE_TYPE_CUBE,
                    dynamicsWorld,
                    (int)box
                );

                rigidBody->SetRestitution(2.0f);
                rigidBody->SetRigidbodyScale(glm::vec3(1.5f, 1.5f, 1.5f));

                box->Transform.Translate(glm::vec3(i * 3, i * 3, i * 3));
                box->Transform.Rotate(glm::vec3(0.1f, 90.1f, 15.1f));

                box->SetID(reinterpret_cast<int>(box));
                box->SetName("box1");

                box->Transform.Scale(glm::vec3(1.5f, 1.5f, 1.5f));

                box->ApplyEntityTransformToRigidbody();
                entities->push_back(box);
            }

        }

        GameObject* gameObject2 = new GameObject;
        gameObject2->AddComponent<Mesh>();
        gameObject2->AddComponent<RigidBody>();

        mesh = gameObject2->pGetComponent<Mesh*>();
        rigidBody = gameObject2->pGetComponent<RigidBody*>();

        mesh->CreateMesh("CoreAssets/ceramic.obj");
        rigidBody->CreateRigidBody(RIGIDBODY_SHAPE_TYPE_PLANE,
            dynamicsWorld,
            reinterpret_cast<int>(gameObject2)
        );

        gameObject2->SetID(reinterpret_cast<int>(gameObject2));
        gameObject2->SetName("Arena");

        gameObject2->Transform.Scale(glm::vec3(228.0f, 25.0f, 329.0f));

        rigidBody->SetMass(0.0f);
        rigidBody->SetRigidbodyScale(gameObject2->Transform.GetScaleValue());


        gameObject2->ApplyEntityTransformToRigidbody();
        entities->push_back(gameObject2);

        GameObject* obj = new GameObject;
        obj->SetID(reinterpret_cast<int>(obj));
        obj->SetName("Test");
        entities->push_back(obj);

        DirectionalLightObject* dlight = new DirectionalLightObject;
        dlight->pGetDirectionalLightUniformData()->lightDirection = glm::vec3(1, -1, 1);
        dlight->pGetDirectionalLightUniformData()->lightColor = glm::vec3(1, 1, 1);

        dlight->SetID(reinterpret_cast<int>(dlight));
        entities->push_back(dlight);
        directionalLightAttributes->push_back(dlight->pGetDirectionalLightUniformData());


        /*DirectionalLightObject* dlight2 = new DirectionalLightObject;
        dlight2->pGetDirectionalLightUniformData()->lightDirection = glm::vec3(0.25, -1, 0.25);
        dlight2->pGetDirectionalLightUniformData()->lightColor = glm::vec3(0.2, 0.2, 0.2);

        dlight2->SetID(reinterpret_cast<int>(dlight2));
        entities->push_back(dlight2);


        directionalLightAttributes->push_back(dlight2->pGetDirectionalLightUniformData());*/

        PointLightObject* pointLight = new PointLightObject;
        pointLight->Transform.Translate(glm::vec3(1.0f, 585.0f, 0.0f));
        pointLight->SetID(reinterpret_cast<int>(pointLight));
        entities->push_back(pointLight);

        PointLightObject* pointLight2 = new PointLightObject;
        pointLight2->Transform.Translate(glm::vec3(25.0f, 3.0f, 13.0f));
        pointLight2->pGetPointLightUniformData()->diffuse = 100;
        pointLight2->SetID(reinterpret_cast<int>(pointLight2));
        entities->push_back(pointLight2);


        PointLightObject* pointLight3 = new PointLightObject;
        pointLight3->Transform.Translate(glm::vec3(80.0f, 3.0f, 13.0f));
        pointLight3->pGetPointLightUniformData()->diffuse = 100;
        pointLight3->SetID(reinterpret_cast<int>(pointLight3));
        entities->push_back(pointLight3);

       

        pointLightAttributes->push_back(pointLight->pGetPointLightUniformData());
        pointLightAttributes->push_back(pointLight2->pGetPointLightUniformData());
        pointLightAttributes->push_back(pointLight3->pGetPointLightUniformData());
       
    }

}
