#pragma once
#include	"../vendor/glm/glm.hpp"
#include	"../vendor/glm/gtc/matrix_transform.hpp"
#include    "Events.h"
#include	"DataTypes.h"
#include	<GLFW/glfw3.h>

namespace Lava{
	class Camera{
	protected:
	// Векторы, описывающие положение
	// и вид из камеры
		glm::vec3 CameraPos;
		glm::vec3 CameraUp;
		glm::vec3 CameraFront;
	// Матрицы преобразований
		glm::mat4 ProjectionMatrix;
		float     FOV;
		double    Yaw,
				  Pitch,
				  CursorLastX,
				  CursorLastY;

	public:
		bool	  CursorFirstMouse;

		Camera() {
			// Инициализация членов класса
			{
				CursorFirstMouse = true;
				CameraPos = glm::vec3(0.0f, 6.0f, 0.0f);
				CameraFront = glm::vec3(0.0f, -2.0f, -1.0f);
				CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
				Yaw = 90.0f;
				Pitch = -30.0f;
				FOV = 80.f;
				CursorLastX = Lava::gWidth / 2;
				CursorLastY = Lava::gHeight / 2;
				glm::vec3 direction;
				direction.x = glm::cos(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
				direction.y = glm::sin(glm::radians((float)Pitch));
				direction.z = glm::sin(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
				CameraFront = glm::normalize(direction);
				ProjectionMatrix = glm::perspective(glm::radians(FOV), ((float)Lava::gWidth / Lava::gHeight), 0.1f, 1000.0f);
			}
		}
		
		glm::mat4 GetView() {
			return glm::lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
		}

		glm::mat4 GetCubemapViewForVulkan() {
			glm::vec3 cubemapFront = CameraFront;
					  cubemapFront.y *= -1;
			return glm::lookAt(CameraPos, (CameraPos + cubemapFront), CameraUp);
		}

		glm::mat4 GetProjectionMatrix() {
			return ProjectionMatrix;
		}

		glm::vec3 GetPosition() {
			return CameraPos;
		}

		virtual void Update(GLFWwindow* window, double deltaTime) {}
	
	};

	class DebugCamera : public Camera {// Камера для просмотры сцены (Отладочная)
		void MouseUpdate() {
			if (keyPressedEventHandler.IsKeyPressed(LAVA_KEY_ALT)) {
				showCursorEventHandler.HideCursor();
				double xpos = mouseMoveEventHandler.GetCursorPos().x,
					   ypos = mouseMoveEventHandler.GetCursorPos().y;


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

				direction.x = glm::cos(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
				direction.y = glm::sin(glm::radians((float)Pitch));
				direction.z = glm::sin(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
				CameraFront = glm::normalize(direction);
			}
			else {
				showCursorEventHandler.ShowCursor();
				CursorFirstMouse = true;
			}

		}
	public:
		DebugCamera() {
			//Инициализация членов класса
			{
				CursorFirstMouse = true;
				CameraPos = glm::vec3(0.0f, 6.0f, -15.0f);
				CameraFront = glm::vec3(0.0f, -2.0f, -1.0f);
				CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
				Yaw = 90.0f;
				Pitch = -30.0f;
				FOV = 80.f;
				ProjectionMatrix = glm::perspective(glm::radians(FOV), ((float)Lava::gWidth / Lava::gHeight), 0.1f, 1000.0f);
				CursorLastX = Lava::gWidth / 2;
				CursorLastY = Lava::gHeight / 2;
				glm::vec3 direction;
				direction.x = glm::cos(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
				direction.y = glm::sin(glm::radians((float)Pitch));
				direction.z = glm::sin(glm::radians((float)Yaw)) * glm::cos(glm::radians((float)Pitch));
				CameraFront = glm::normalize(direction);
			}
			
		}
		
		void Update(double deltaTime) {
			if (ENABLE_DYNAMIC_VIEWPORT) {
				ProjectionMatrix = glm::perspective(glm::radians(FOV), ((float)gEditor3DView.width / gEditor3DView.height), 0.1f, 1000.0f);
			}

			float moveSpeed = 5.25f;
			float sprintSpeed = 2.f;

			MouseUpdate();
			
			if (keyPressedEventHandler.IsKeyPressed(LAVA_KEY_A)) {
				CameraPos -= glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)deltaTime;
			}
			if (keyPressedEventHandler.IsKeyPressed(LAVA_KEY_A) && keyPressedEventHandler.IsKeyPressed(LAVA_KEY_G)) {
				CameraPos -= glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)deltaTime * sprintSpeed;
			}
			if (keyPressedEventHandler.IsKeyPressed(LAVA_KEY_D)) {
				CameraPos += glm::normalize(glm::cross(CameraFront, CameraUp)) * moveSpeed * (float)deltaTime;
			}
			/*if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
				_CameraPos += glm::normalize(glm::cross(_CameraFront, _CameraUp)) * moveSpeed * (float)deltaTime * sprintSpeed;
			}*/
			if (keyPressedEventHandler.IsKeyPressed(LAVA_KEY_W)) {
				CameraPos += moveSpeed * CameraFront * (float)deltaTime;
			}
			/*if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
				_CameraPos += moveSpeed * _CameraFront * (float)deltaTime * sprintSpeed;
			}*/
			if (keyPressedEventHandler.IsKeyPressed(LAVA_KEY_S)) {
				CameraPos -= moveSpeed * CameraFront * (float)deltaTime;
			}
			/*if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
				_CameraPos -= moveSpeed * _CameraFront * (float)deltaTime * sprintSpeed;
			}*/
		}
	}debugCamera;

}
