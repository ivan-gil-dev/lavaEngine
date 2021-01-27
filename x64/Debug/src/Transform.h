#pragma once
#include	"../vendor/glm/glm.hpp"
#include	"../vendor/glm/gtx/quaternion.hpp"

namespace Lava{
	class Transform {
		glm::mat4 scaleMatrix{}, translationMatrix{}, rotationMatrix{};
		glm::vec3 position;
		glm::vec3 rotationAngles;
		glm::vec3 scaleValue;
		glm::vec3 initPosition;
		glm::vec3 initRotationAngles;
		glm::quat quaternion;
	public:
		void translate(glm::vec3 translation) {
			position = translation;
			translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
		}

		void setInitialTransform() {
			position = initPosition;
			rotationAngles = initRotationAngles;
			setRotation(rotationAngles);
			translate(position);
		}

		void setRotation(glm::vec3 Rotation) {
			glm::quat quatX, quatY, quatZ;
			rotationAngles.x = Rotation.x;
			quatX = glm::angleAxis(glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));

			rotationAngles.y = Rotation.y;
			quatY = glm::angleAxis(glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));

			rotationAngles.z = Rotation.z;
			quatZ = glm::angleAxis(glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			quaternion = quatZ * quatY * quatX;
			rotationMatrix = glm::toMat4(quaternion);
		}

		void setQuaternion(glm::quat quat) {
			quaternion = quat;
			rotationMatrix = glm::toMat4(quaternion);
		}

		void scale(glm::vec3 ScaleValue) {
			scaleValue = ScaleValue;
			scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleValue.x, scaleValue.y, scaleValue.z));
		}

		void initTranslation(glm::vec3 translation) {
			position = translation;
			initPosition = position;
			translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
		}

		void initRotation(glm::vec3 Rotation) {
			glm::quat quatX, quatY, quatZ;
			rotationAngles.x = Rotation.x;
			quatX = glm::angleAxis(glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));

			rotationAngles.y = Rotation.y;
			quatY = glm::angleAxis(glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));

			rotationAngles.z = Rotation.z;
			quatZ = glm::angleAxis(glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			quaternion = quatZ * quatY * quatX;
			rotationMatrix = glm::toMat4(quaternion);
			initRotationAngles = rotationAngles;
		}

		glm::vec3 getRotationAngles() {
			return rotationAngles;
		}

		glm::vec3 getPosition() {
			return position;
		}

		glm::vec3 getScale() {
			return scaleValue;
		}

		glm::quat getQuaternion() {
			return quaternion;
		}

		glm::mat4 getMatrixProduct() {
			return translationMatrix * rotationMatrix * scaleMatrix;
		}

		Transform() {
			position = { 0.0f,0.0f,0.0f };
			scaleValue = { 1.0f,1.0f,1.0f };
			quaternion = { 0.0f,0.0f,0.0f,0.0f };
			rotationAngles = { 0.0f,0.0f,0.0f };

			scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleValue.x, scaleValue.y, scaleValue.z));
			rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
		}
	};

}
