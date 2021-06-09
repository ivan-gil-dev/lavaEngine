#include "../Headers/Entities.h"
#include "../Headers/Events.h"
#include "../Headers/Renderer/Renderer.h"

//void Engine::Camera::SetPosition(glm::vec3 Pos)
//{
//    //CameraPos = glm::vec3(0);
//
//    CameraPos += Pos.x / CameraFront;
//    CameraPos += Pos.y / CameraUp;
//    CameraPos += Pos.z / glm::normalize(glm::cross(CameraFront, CameraUp));
//
//    std::cout << CameraPos.x << " " << CameraPos.y << " " << CameraPos.z << " " << std::endl;
//}

Engine::Camera::Camera()
{
    CursorFirstMouse = true;

    CameraPos = glm::vec3(0.0f, 6.0f, 0.0f);
    CameraFront = glm::vec3(0.0f, -2.0f, -1.0f);
    CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    CursorLastX = Engine::Globals::gWidth / 2;
    CursorLastY = Engine::Globals::gHeight / 2;

    glm::vec3 direction;

    Yaw = 90.0f;
    Pitch = -30.0f;
    FOV = 80.f;

    direction.x = glm::cos(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
    direction.y = glm::sin(glm::radians((float)Pitch));
    direction.z = glm::sin(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));

    CameraFront = glm::normalize(direction);

    ProjectionMatrix = glm::perspective(glm::radians(FOV), ((float)Engine::Globals::gWidth / Engine::Globals::gHeight), 0.1f, 1000.0f);
}

void Engine::Camera::SetCameraPos(glm::vec3 pos)
{
    CameraPos = pos;
}

void Engine::Camera::SetCameraFront(glm::vec3 front)
{
    CameraFront = front;
}

Engine::DataTypes::ViewProjection_t Engine::Camera::GetViewProjectionForEntity() {
    DataTypes::ViewProjection_t viewProjection;
    viewProjection.view = glm::lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
    if (ENABLE_DYNAMIC_VIEWPORT) {
        viewProjection.projection = glm::perspective(glm::radians(FOV), ((float)renderer.rendererViewport.width / renderer.rendererViewport.height), 0.1f, 100000.0f);
    }
    else {
        viewProjection.projection = glm::perspective(glm::radians(FOV), ((float)Engine::Globals::gWidth / Engine::Globals::gHeight), 0.1f, 100000.0f);
    }
    return viewProjection;
}

Engine::DataTypes::ViewProjection_t Engine::Camera::GetViewProjectionForCubemap() {
    DataTypes::ViewProjection_t viewProjection;

    glm::vec3 cubemapFront = CameraFront;
    cubemapFront.y *= -1;//�������� �� (-1) ��� ����������� ����������� skybox'�

    viewProjection.view = glm::lookAt(CameraPos, CameraPos + cubemapFront, CameraUp);

    if (ENABLE_DYNAMIC_VIEWPORT) {
        viewProjection.projection = glm::perspective(glm::radians(FOV), ((float)renderer.rendererViewport.width / renderer.rendererViewport.height), 0.1f, 1000.0f);
    }
    else {
        viewProjection.projection = glm::perspective(glm::radians(FOV), ((float)Engine::Globals::gWidth / Engine::Globals::gHeight), 0.1f, 1000.0f);
    }
    return viewProjection;
}

glm::vec3 Engine::Camera::GetPosition()
{
    return CameraPos;
}

glm::vec3 Engine::Camera::GetCameraFront()
{
    return CameraFront;
}

void Engine::Camera::Update()
{
}

void Engine::EditorCamera::MouseUpdate()
{
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

        glm::vec3 direction;

        {
            direction.x = glm::cos(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
            direction.y = glm::sin(glm::radians((float)Pitch));
            direction.z = glm::sin(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
            CameraFront = glm::normalize(direction);
        }
    }
    else {
        Globals::showCursorEventHandler.ShowCursor();
        CursorFirstMouse = true;
    }
}

Engine::EditorCamera::EditorCamera()
{
    {
        CursorFirstMouse = true;
        CameraPos = glm::vec3(0.0f, 6.0f, -15.0f);
        CameraFront = glm::vec3(0.0f, -2.0f, -1.0f);
        CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = 90.0f;
        Pitch = -30.0f;
        FOV = 80.f;
        ProjectionMatrix = glm::perspective(glm::radians(FOV), ((float)Engine::Globals::gWidth / Engine::Globals::gHeight), 0.1f, 1000.0f);
        CursorLastX = Engine::Globals::gWidth / 2;
        CursorLastY = Engine::Globals::gHeight / 2;
        glm::vec3 direction;
        direction.x = glm::cos(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
        direction.y = glm::sin(glm::radians((float)Pitch));
        direction.z = glm::sin(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
        CameraFront = glm::normalize(direction);
    }
}

void Engine::EditorCamera::Reset()
{
    CameraPos = glm::vec3(0.0f, 6.0f, -15.0f);
    CameraFront = glm::vec3(0.0f, -2.0f, -1.0f);
    CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    Yaw = 90.0f;
    Pitch = -30.0f;
    glm::vec3 direction;
    direction.x = glm::cos(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
    direction.y = glm::sin(glm::radians((float)Pitch));
    direction.z = glm::sin(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
    CameraFront = glm::normalize(direction);
}

void Engine::EditorCamera::Update()
{
    MouseUpdate();
    float step = 1.0f;
    //����������� ������
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_ALT))
    {
        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_A) && !Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
            CameraPos -= glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)Engine::Globals::DeltaTime;
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_A) && Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
            CameraPos -= glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)Engine::Globals::DeltaTime * sprintSpeed;
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_D) && !Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
            CameraPos += glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)Engine::Globals::DeltaTime;
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_D) && Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
            CameraPos += glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)Engine::Globals::DeltaTime * sprintSpeed;
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_W) && !Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
            CameraPos += moveSpeed * CameraFront * (float)Engine::Globals::DeltaTime;
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_W) && Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
            CameraPos += moveSpeed * CameraFront * (float)Engine::Globals::DeltaTime * sprintSpeed;
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_S) && !Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
            CameraPos -= moveSpeed * CameraFront * (float)Engine::Globals::DeltaTime;
        }
        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_S) && Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
            CameraPos -= moveSpeed * CameraFront * (float)Engine::Globals::DeltaTime * sprintSpeed;
        }

        if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_O) && !Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
            CameraPos += CameraUp * moveSpeed * (float)Engine::Globals::DeltaTime;
        }
    }
}

Engine::PointLightObject::PointLightObject() {
    pointLightSettings.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    pointLightSettings.lightPosition = glm::vec3(1.0f, 12.0f, 1.0f);

    pointLightSettings.constant = 1.0f;
    pointLightSettings.linear = 0.09f;
    pointLightSettings.quadrantic = 0.032f;

    pointLightSettings.ambient = 1.f;
    pointLightSettings.diffuse = 1.f;
    pointLightSettings.specular = 1.f;

    Type = ENTITY_TYPE_POINTLIGHT_OBJECT;
    Name = "PointLight";
}

void Engine::PointLightObject::UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera) {
    pointLightSettings.lightPosition = Transform.GetPosition();
}

Engine::DataTypes::PointLightAttributes_t* Engine::PointLightObject::pGetPointLightUniformData() {
    return &pointLightSettings;
}

Engine::PointLightObject::~PointLightObject() {
}

void Engine::Entity::lua_Test(std::string testVar)
{
    std::cout << testVar << std::endl;
}

void Engine::Entity::lua_MoveEntity(float x, float y, float z)
{
    Transform.SetTranslation(
        glm::vec3(
            Transform.GetPosition().x + x,
            Transform.GetPosition().y + y,
            Transform.GetPosition().z + z
        )
    );
}

void Engine::Entity::lua_RotateEntity(float x, float y, float z)
{
    Transform.SetRotation(
        glm::vec3(
            Transform.GetEulerAngles().x + x,
            Transform.GetEulerAngles().y + y,
            Transform.GetEulerAngles().z + z
        )
    );
}

Engine::Entity::Entity() {
    ID = 0;
    Name = "Entity";
}

Engine::Entity::~Entity() {
}

void Engine::Entity::ExecuteScript()
{
    script.doScriptUpdate(this);
}

Engine::Lua::Script* Engine::Entity::pGetScript()
{
    return &script;
}

void Engine::Entity::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
}

void Engine::Entity::Start() {
}

void Engine::Entity::Update() {
    //script.doScript();
}

void Engine::Entity::SetRef(Entity* reference)
{
    ref = reference;
}

void Engine::Entity::SetName(std::string name) {
    Name = name;
}

void Engine::Entity::SetID(uint64_t Id) {
    ID = Id;
}

Engine::EntityType Engine::Entity::GetEntityType() {
    return Type;
}

std::string Engine::Entity::GetName() {
    return Name;
}

uint64_t Engine::Entity::GetID() {
    return ID;
}

template <typename T> void AddComponent() {
}

template <> void Engine::GameObject::AddComponent<Engine::RigidBody>() {
    if (!IsRigidbodyCreated) pRigidBody = new RigidBody;
    else {
        delete pRigidBody;
        pRigidBody = new RigidBody;
        IsRigidbodyCreated = true;
    }
}

template <> void Engine::GameObject::AddComponent<Engine::Mesh>() {
    if (!IsMeshCreated) pMesh = new Mesh;
    else {
        delete pMesh;
        pMesh = new Mesh;
        IsMeshCreated = true;
    }
}

template <typename T> T Engine::GameObject::pGetComponent() {
}

template <> Engine::RigidBody* Engine::GameObject::pGetComponent<Engine::RigidBody*>() {
    return pRigidBody;
}

template <> Engine::Mesh* Engine::GameObject::pGetComponent<Engine::Mesh*>() {
    return pMesh;
}

template <typename T> void Engine::GameObject::DeleteComponent() {
}

template <> void Engine::GameObject::DeleteComponent<Engine::RigidBody>() {
    pRigidBody->Destroy(Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld);
    delete pRigidBody;
    IsRigidbodyCreated = false;
}

template <> void Engine::GameObject::DeleteComponent<Engine::Mesh>() {
    pMesh->Destroy();
    delete pMesh;
    IsMeshCreated = false;
}

Engine::GameObject::GameObject() {
    Type = ENTITY_TYPE_GAME_OBJECT;
    Name = "Game Object";
    ID = 0;
}

void Engine::GameObject::UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera,
    std::vector<DataTypes::PointLightAttributes_t*> spotlightAttributes,
    std::vector<DataTypes::DirectionalLightAttributes_t*> directionalLightAttributes) {
    if (pRigidBody != nullptr) {
        pRigidBody->pGetDebugMesh()->UpdateUniforms(imageIndex, device, camera.GetViewProjectionForEntity());
    }
    if (pMesh != nullptr) {
        pMesh->UpdateUniforms(imageIndex, device, camera.GetPosition(), camera.GetViewProjectionForEntity(), Transform.GetMatrixProduct(), spotlightAttributes, directionalLightAttributes);
    }
}

void Engine::GameObject::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
    if (ENABLE_RIGIDBODY_MESH && Globals::states.showRigidbodyMeshes && pRigidBody != nullptr) {
        pRigidBody->pGetDebugMesh()->Draw(commandBuffer, imageIndex, renderer.graphicsPipelineForRigidBodyMesh.Get());
    }
    if (Globals::states.showMeshes && pMesh != nullptr) {
        pMesh->Draw(commandBuffer, imageIndex, renderer.graphicsPipelineForMesh.Get());
    }
}

void Engine::GameObject::ExecuteScript()
{
    script.doScriptUpdate(this);
}

void Engine::GameObject::DrawShadowMaps(VkCommandBuffer commandBuffer, int imageIndex, std::vector<VkDescriptorSet>& pDescriptorSets)
{
    if (Globals::states.showMeshes && pMesh != nullptr) {
        pMesh->DrawShadowMaps(commandBuffer, imageIndex, pDescriptorSets);
    }
}

Engine::GameObject::~GameObject() {
    if (pMesh != nullptr) {
        pMesh->Destroy();
        delete pMesh;
    }

    if (pRigidBody != nullptr) {
        pRigidBody->Destroy(Engine::Globals::bulletPhysicsGlobalObjects.dynamicsWorld);
        delete pRigidBody;
    }
}

void Engine::GameObject::ApplyPhysicsToEntity() {
    if (pRigidBody != 0) {
        if (pRigidBody->GetBulletRigidBody()->getMass() != 0) {
            //��������� ������������� �� ���������� ������
            btTransform worldTransform;
            worldTransform = pRigidBody->GetBulletRigidBody()->getCenterOfMassTransform();

            //��������� ������������� � ����������� ������
            glm::vec3 position = glm::vec3(
                worldTransform.getOrigin().getX(),
                worldTransform.getOrigin().getY(),
                worldTransform.getOrigin().getZ()
            );

            Transform.SetTranslation(glm::vec3(position.x, position.y, position.z));

            glm::quat quaternion = glm::quat(
                (float)worldTransform.getRotation().getX(),
                (float)worldTransform.getRotation().getY(),
                (float)worldTransform.getRotation().getZ(),
                (float)worldTransform.getRotation().getW()
            );

            Transform.SetQuaternion(quaternion);

            if (pRigidBody->pGetDebugMesh() != nullptr) {
                pRigidBody->pGetDebugMesh()->Transform.SetTranslation(position);
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

    cubemapPaths = paths;

    pMesh = new CubemapMesh;
    pMesh->CreateCubemapMesh(paths);
}

void Engine::CubemapObject::Draw(VkCommandBuffer commandBuffer, int imageIndex) {
    if (Globals::states.showSkybox && pMesh != nullptr) {
        pMesh->Draw(commandBuffer, imageIndex, renderer.graphicsPipelineForCubemapObjects.Get());
    }
}

void Engine::CubemapObject::UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera) {
    if (pMesh != nullptr) {
        pMesh->UpdateUniforms(imageIndex, device, camera.GetViewProjectionForCubemap());
    }
}

std::vector<std::string> Engine::CubemapObject::GetCubemapPaths() {
    return cubemapPaths;
}

Engine::CubemapObject::~CubemapObject() {
    if (pMesh != nullptr) {
        pMesh->Destroy();
        delete pMesh;
    }
}

Engine::DirectionalLightObject::DirectionalLightObject()
{
    Type = ENTITY_TYPE_DIRECTIONAL_LIGHT_OBJECT;
    Name = "Directional Light";
    directionalLightSettings.lightColor = glm::vec3(255.0f, 255.0f, 255.0f);
    directionalLightSettings.lightDirection = glm::vec3(0.25, 1, 0.25);

    directionalLightSettings.ambient = 1.0f;
    directionalLightSettings.diffuse = 1.0f;
    directionalLightSettings.specular = 1.0f;
}

void Engine::DirectionalLightObject::UpdateUniforms(uint32_t imageIndex, VkDevice device, Camera camera)
{
}

Engine::DataTypes::DirectionalLightAttributes_t* Engine::DirectionalLightObject::pGetDirectionalLightUniformData()
{
    return &directionalLightSettings;
}

Engine::DirectionalLightObject::~DirectionalLightObject()
{
}