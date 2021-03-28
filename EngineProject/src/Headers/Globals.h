#ifndef globals_h
#define globals_h

#define GLM_ENABLE_EXPERIMENTAL

#include <btBulletDynamicsCommon.h>
#include "../../vendor/glm/gtx/hash.hpp"
#include "../../vendor/volk.h"
#include "../../vendor/glm/glm.hpp"
#include <spdlog/spdlog.h>
#include <vector>
#include <iostream>
#include <windows.h>


#ifdef _DEBUG
static bool ENABLE_DYNAMIC_VIEWPORT = true;
static bool ENABLE_IMGUI = true;
static bool ENABLE_RIGIDBODY_MESH = true;
#else
static bool ENABLE_DYNAMIC_VIEWPORT = false;
static bool ENABLE_IMGUI = false;
static bool ENABLE_RIGIDBODY_MESH = false;
#endif

namespace Engine{
	namespace Globals{
		#define MAX_SPOTLIGHTS 10
		extern bool  gToggleFullscreen;
		extern bool  gShowMeshes;
		extern bool  gShowRigidbodyMeshes;
		extern bool  gShowSkybox;
		extern int   gHeight,
					 gWidth;
		extern bool  gIsScenePlaying;
		extern const int gmax_frames;
		extern VkSampleCountFlagBits gMSAAsamples;

		extern std::shared_ptr<spdlog::logger> gLogger;

		extern btBroadphaseInterface* gBroadphase;
		extern btDefaultCollisionConfiguration* gCollisionConfiguration;
		extern btCollisionDispatcher* gDispatcher;
		extern btSequentialImpulseConstraintSolver* gSolver;
		extern btDynamicsWorld* gDynamicsWorld;

		extern VkRect2D   gEditor3DScissors;
		extern VkViewport gEditor3DView;
		extern double DeltaTime;
	}
}

#endif