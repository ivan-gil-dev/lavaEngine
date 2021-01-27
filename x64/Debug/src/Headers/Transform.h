#pragma once
#include	"../vendor/glm/glm.hpp"
#include	"../vendor/glm/gtx/quaternion.hpp"

namespace Lava{
	class Transform {
		glm::mat4 ScaleMatrix{}, 
				  TranslationMatrix{},
				  RotationMatrix{};

		glm::vec3 Position;
		glm::vec3 EulerAngles;
		glm::quat Quaternion;
		glm::vec3 ScaleValue;

		glm::vec3 StartPosition;
		glm::vec3 StartRotationAngles;
		
	public:
		void Translate(glm::vec3 translation) {
			Position += translation;
			TranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
		}

		void ResetTransform() {
			Position = StartPosition;
			EulerAngles = StartRotationAngles;
			Rotate(EulerAngles);
			Translate(Position);
			
		}

		void Rotate(glm::vec3 Rotation) {
			glm::quat quatX, quatY, quatZ;

			EulerAngles.x += Rotation.x;
			quatX = glm::angleAxis(glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));

			EulerAngles.y += Rotation.y;
			quatY = glm::angleAxis(glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));

			EulerAngles.z += Rotation.z;
			quatZ = glm::angleAxis(glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			Quaternion = quatZ * quatY * quatX;

			RotationMatrix = glm::toMat4(Quaternion);
		}

		void SetQuaternion(glm::quat quat) {
			Quaternion = quat;
			RotationMatrix = glm::toMat4(Quaternion);
		}

		void Scale(glm::vec3 ScaleValue) {
			ScaleValue = ScaleValue;
			ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(ScaleValue.x, ScaleValue.y, ScaleValue.z));
		}

		void SetStartTranslation(glm::vec3 translation) {
			Position = translation;
			StartPosition = Position;
			TranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
		}

		void SetStartRotation(glm::vec3 Rotation) {
			glm::quat quatX, quatY, quatZ;
			EulerAngles.x = Rotation.x;
			quatX = glm::angleAxis(glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));

			EulerAngles.y = Rotation.y;
			quatY = glm::angleAxis(glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));

			EulerAngles.z = Rotation.z;
			quatZ = glm::angleAxis(glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			Quaternion = quatZ * quatY * quatX;
			RotationMatrix = glm::toMat4(Quaternion);
			StartRotationAngles = EulerAngles;
		}

		glm::vec3 GetEulerAngles() {
			return EulerAngles;
		}

		glm::vec3 GetPosition() {
			return Position;
		}

		glm::vec3 GetScale() {
			return ScaleValue;
		}

		glm::quat GetQuaternion() {
			return Quaternion;
		}

		glm::mat4 GetMatrixProduct() {
			return TranslationMatrix * RotationMatrix * ScaleMatrix;
		}

		Transform() {
			Position = { 0.0f,0.0f,0.0f };
			ScaleValue = { 1.0f,1.0f,1.0f };
			Quaternion = { 0.0f,0.0f,0.0f,0.0f };
			EulerAngles = { 0.0f,0.0f,0.0f };

			ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(ScaleValue.x, ScaleValue.y, ScaleValue.z));
			RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			TranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, Position.z));
		}
	};

}
