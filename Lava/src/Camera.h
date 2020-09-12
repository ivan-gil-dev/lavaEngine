#pragma once
#include "../vendor/glm/glm.hpp"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "DataTypes.h"
#include <GLFW/glfw3.h>
namespace Lava{
	class Camera {
		glm::vec3 cameraPos;
		glm::vec3 cameraUp;
		glm::mat4 projectionMatrix;
		double yaw, pitch;
		glm::vec3 cameraFront;
	public:
		bool firstMouse;
		Camera() {
			firstMouse = true;
			cameraPos = glm::vec3(0.0f, 6.0f, -15.0f);
			cameraFront = glm::vec3(0.0f, -2.0f, -1.0f);
			cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
			yaw = 90.0f;
			pitch = -30.0f;

			projectionMatrix =  glm::perspective(glm::radians(80.0f), ((float)WIDTH /HEIGHT), 0.1f, 1000.0f);

			glm::vec3 direction;
			direction.x = glm::cos(glm::radians((float)yaw)) * glm::cos(glm::radians((float)pitch));
			direction.y = glm::sin(glm::radians((float)pitch));
			direction.z = glm::sin(glm::radians((float)yaw)) * glm::cos(glm::radians((float)pitch));
			cameraFront = glm::normalize(direction);
		}
		glm::mat4 getView() {
			return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		}
		glm::mat4 getCubemapView(){
			glm::vec3 cubemapFront = cameraFront;
			cubemapFront.y *= -1;
			return glm::lookAt(cameraPos, (cameraPos + cubemapFront), cameraUp);
		}
		glm::vec3 getPosition() {
			return cameraPos;
		}
		void cursorUpdate(double xoffset, double yoffset) {
			float sensitivity = 0.1f;
			xoffset *= sensitivity;
			yoffset *= sensitivity;

			yaw += xoffset;
			pitch += yoffset;

			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			glm::vec3 direction;

			direction.x = glm::cos(glm::radians((float)yaw)) * glm::cos(glm::radians((float)pitch));
			direction.y = glm::sin(glm::radians((float)pitch));
			direction.z = glm::sin(glm::radians((float)yaw)) * glm::cos(glm::radians((float)pitch));
			cameraFront = glm::normalize(direction);
		}
		void processInput(GLFWwindow* window, double deltaTime) {
			float moveSpeed = 6.25f;
			float fasterMoveSpeed = 10.f;
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
				cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * moveSpeed * (float)deltaTime;
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
				cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * fasterMoveSpeed * (float)deltaTime;
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
				cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * moveSpeed * (float)deltaTime;
			}
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				cameraPos += moveSpeed * cameraFront * (float)deltaTime;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				cameraPos -= moveSpeed * cameraFront * (float)deltaTime;
			}
			
		}
		glm::mat4 getProjectionMatrix(){
			return projectionMatrix;
		}
	}camera;
}
