#pragma once

#include	"../../vendor/glm/glm.hpp"
#include	"../../vendor/glm/gtc/matrix_transform.hpp"
#include    "Events.h"
#include	"Renderer/DataTypes.h"

namespace Engine{
	class Camera{
	protected:
		glm::vec3 CameraPos;
		glm::vec3 CameraUp;
		glm::vec3 CameraFront;
		glm::mat4 ProjectionMatrix;
		float     FOV;
		double    Yaw,
				  Pitch,
				  CursorLastX,
				  CursorLastY;
	public:
		bool	  CursorFirstMouse;

		Camera();
		
		glm::mat4 GetView();

		glm::mat4 GetCubemapViewForVulkan();

		glm::mat4 GetProjectionMatrix();

		glm::vec3 GetPosition();

		virtual void Update();
	
	};

	class EditorCamera : public Camera {

		const float moveSpeed = 5.25f;

		const float sprintSpeed = 2.f;

		void MouseUpdate();
	public:
		EditorCamera();
		
		void Update();
	};

	

}
