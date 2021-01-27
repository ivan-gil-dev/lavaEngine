#pragma once
#include	"../vendor/glm/glm.hpp"
#include	"../vendor/glm/gtx/quaternion.hpp"
#include    "Globals.h"
namespace Lava{
	class Transform {
	public:
		// Матрицы преобразований
		glm::mat4 ScaleMatrix{}, 
				  TranslationMatrix{},
				  RotationMatrix{};
	public:
		glm::vec3 Position;
		glm::vec3 EulerAngles;
		glm::quat Quaternion;
		glm::vec3 ScaleValue;
	public:
		glm::vec3 StartTranslation;
		glm::vec3 StartRotation;
		glm::vec3 StartScale;
		
	public:
		void Translate(glm::vec3 translation) {
			Position = translation;
			TranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));

			if (!gIsScenePlaying){
				StartTranslation = translation;
			}
		}

		void Rotate(glm::vec3 Rotation) {
			
			Quaternion = glm::quat(
				glm::vec3(
					glm::radians(Rotation.x),
					glm::radians(Rotation.y),
					glm::radians(Rotation.z)
				
				)
			);

			RotationMatrix = glm::toMat4(Quaternion);

			EulerAngles = Rotation;

			if (!gIsScenePlaying) {
				StartRotation = Rotation;
			}
		}

		void Scale(glm::vec3 ScaleVal) {
			ScaleValue = ScaleVal;
			ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(ScaleVal.x, ScaleVal.y, ScaleVal.z));
			if (!gIsScenePlaying) {
				StartScale = ScaleVal;
			}
		}

		void ResetTransform() {
			Position = StartTranslation;
			EulerAngles = StartRotation;
			ScaleValue = StartScale;
			Translate(StartTranslation);
			Rotate(StartRotation);
			Scale(ScaleValue);
		}

		void sQuaternion(glm::quat quat) {
			Quaternion = quat;
			EulerAngles = glm::eulerAngles(quat);
			RotationMatrix = glm::toMat4(Quaternion);
		}

		glm::vec3 gEulerAngles() {
			return EulerAngles;
		}

		glm::vec3 gPosition() {
			return Position;
		}

		glm::vec3 gScaleValue() {
			return ScaleValue;
		}

		glm::quat gQuaternion() {
			return Quaternion;
		}

		glm::mat4 gMatrixProduct() {
			return TranslationMatrix * RotationMatrix * ScaleMatrix;
		}

		Transform() {
			Position = { 0.0f,0.0f,0.0f };
			ScaleValue = { 1.0f,1.0f,1.0f };
			Quaternion = { 0.0f,0.0f,0.0f,0.0f };
			EulerAngles = { 0.0f,0.0f,0.0f };
			StartTranslation = {0.0f,0.0f,0.0f};
			StartRotation = {0.0f,0.0f,0.0f};
			StartScale = {1.0f,1.0f,1.0f};


			ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(ScaleValue.x, ScaleValue.y, ScaleValue.z));
			RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			TranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, Position.z));
		}
	};

}
