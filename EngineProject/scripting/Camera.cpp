#include "Camera.h"
#include "Renderer/Renderer.h"
#include "Globals.h"

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

glm::mat4 Engine::Camera::GetView()
{
    return glm::lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
}

glm::mat4 Engine::Camera::GetCubemapViewForVulkan()
{
    glm::vec3 cubemapFront = CameraFront;
    cubemapFront.y *= -1;//�������� �� (-1) ��� ����������� ����������� skybox'�

    return glm::lookAt(CameraPos, (CameraPos + cubemapFront), CameraUp);
}

glm::mat4 Engine::Camera::GetProjectionMatrix()
{
    if (ENABLE_DYNAMIC_VIEWPORT) {
        ProjectionMatrix = glm::perspective(glm::radians(FOV), ((float)renderer.rendererViewport.width / renderer.rendererViewport.height), 0.1f, 1000.0f);
    }
    else {
        ProjectionMatrix = glm::perspective(glm::radians(FOV), ((float)Engine::Globals::gWidth / Engine::Globals::gHeight), 0.1f, 1000.0f);
    }
    return ProjectionMatrix;
}

glm::vec3 Engine::Camera::GetPosition()
{
    return CameraPos;
}

void Engine::Camera::Update()
{

}

void Engine::EditorCamera::MouseUpdate()
{
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_ALT)) {
        Globals::showCursorEventHandler.HideCursor();

        double xpos = Globals::mouseMoveEventHandler.GetCursorPos().x,
               ypos = Globals::mouseMoveEventHandler.GetCursorPos().y;


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

        Yaw += xoffset;
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

void Engine::EditorCamera::Update()
{
    MouseUpdate();

    //����������� ������
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_A)) {
        CameraPos -= glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)Engine::Globals::DeltaTime;
    }
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_A) && Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
        CameraPos -= glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)Engine::Globals::DeltaTime * sprintSpeed;
    }
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_D)) {
        CameraPos += glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)Engine::Globals::DeltaTime;
    }
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_D) && Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
        CameraPos += glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)Engine::Globals::DeltaTime * sprintSpeed;
    }
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_W)) {
        CameraPos += moveSpeed * CameraFront * (float)Engine::Globals::DeltaTime;
    }
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_W) && Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
        CameraPos += moveSpeed * CameraFront * (float)Engine::Globals::DeltaTime * sprintSpeed;
    }
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_S)) {
        CameraPos -= moveSpeed * CameraFront * (float)Engine::Globals::DeltaTime;
    }
    if (Globals::keyPressedEventHandler.IsKeyPressed(KEY_S) && Globals::keyPressedEventHandler.IsKeyPressed(KEY_F)) {
        CameraPos -= moveSpeed * CameraFront * (float)Engine::Globals::DeltaTime * sprintSpeed;
    }
}
