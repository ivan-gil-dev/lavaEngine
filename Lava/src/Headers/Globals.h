#pragma once
#define GLM_ENABLE_EXPERIMENTAL


#include	<btBulletDynamicsCommon.h>
#include	<BulletCollision/Gimpact/btGImpactShape.h>
#include	<BulletCollision/CollisionShapes/btShapeHull.h>
#include	<BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include	<../vendor/glm/gtx/hash.hpp>
#include	"../vendor/volk.h"
#include	"../vendor/glm/glm.hpp"
#include	<spdlog/spdlog.h>
#include	<vector>
#include	<iostream>

#include	<windows.h>



#ifdef _DEBUG
static bool ENABLE_DYNAMIC_VIEWPORT = true;
static bool ENABLE_IMGUI = true;
static bool ENABLE_RIGIDBODY_MESH = true;
#else
static bool ENABLE_DYNAMIC_VIEWPORT = false;
static bool ENABLE_IMGUI = false;
static bool ENABLE_RIGIDBODY_MESH = false;
#endif

namespace Lava{
	#define MAX_SPOTLIGHTS 1

	static int   gEditorExtentWidth = 0;
	static int   gEditorExtentHeight = 0;
	static bool  gMiddleMousePressed = false;
	static bool  gToggleFullscreen = false;
	static bool  gShowMeshes = true;
	static bool  gShowRigidbodyMeshes = true;
	static bool  gShowSkybox = true;

	static float gHeight = 600,
			     gWidth  = 800;

	static bool gIsScenePlaying = false;

	static const int gmax_frames = 2;

	static VkSampleCountFlagBits gMSAAsamples = VK_SAMPLE_COUNT_2_BIT;
	static VkSampleCountFlagBits gmaxMSAAsamples;

	static btBroadphaseInterface*			    gBroadphase;
	static btDefaultCollisionConfiguration*	    gCollisionConfiguration;
	static btCollisionDispatcher*				gDispatcher;
	static btSequentialImpulseConstraintSolver* gSolver;
	static btDynamicsWorld*						gDynamicsWorld;

	static VkRect2D   gEditor3DScissors = {};
	static VkViewport gEditor3DView{};
}
