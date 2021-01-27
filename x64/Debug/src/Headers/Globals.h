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
static bool LAVA_GLOBAL_ENABLE_DYNAMIC_VIEWPORT = true;
static bool LAVA_GLOBAL_ENABLE_IMGUI = true;
static bool LAVA_GLOBAL_ENABLE_RIGIDBODY_MESH = true;
#else
static bool LAVA_GLOBAL_ENABLE_DYNAMIC_VIEWPORT = false;
static bool LAVA_GLOBAL_ENABLE_IMGUI = false;
static bool LAVA_GLOBAL_ENABLE_RIGIDBODY_MESH = false;
#endif

namespace Lava{
	
	static bool  gShowCursor = false;
	static int   gEditorExtentWidth = 0;
	static int   gEditorExtentHeight = 0;
	static bool  gMiddleMousePressed = false;

	static float gHeight = 600,
			     gWidth  = 800;

	static const int gmax_frames = 2;

	static VkSampleCountFlagBits gMSAAsamples = VK_SAMPLE_COUNT_2_BIT;
	static VkSampleCountFlagBits gmaxMSAAsamples;

	static btBroadphaseInterface*			    gBroadphase;
	static btDefaultCollisionConfiguration*	    gCollisionConfiguration;
	static btCollisionDispatcher*				gDispatcher;
	static btSequentialImpulseConstraintSolver* gSolver;
	static btDynamicsWorld*						gDynamicsWorld;

	static VkRect2D   gEditor3DScissors = {};
	static VkViewport gEditor3DViewport{};
}
