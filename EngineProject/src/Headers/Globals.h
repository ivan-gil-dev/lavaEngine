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
		struct BulletPhysicsGlobalObjects
		{
            btBroadphaseInterface* broadphase;
            btDefaultCollisionConfiguration* collisionConfiguration;
            btCollisionDispatcher* dispatcher;
            btSequentialImpulseConstraintSolver* solver;
            btDynamicsWorld* dynamicsWorld;

            void InitBullet() {
                broadphase = new btDbvtBroadphase();
                collisionConfiguration = new btDefaultCollisionConfiguration();
                dispatcher = new btCollisionDispatcher(collisionConfiguration);
                solver = new btSequentialImpulseConstraintSolver;
                dynamicsWorld = new btDiscreteDynamicsWorld(
                    dispatcher, broadphase,
                    solver,collisionConfiguration
                );
                dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
            }

            void CleanBullet() {
                delete solver;
                delete dispatcher;
                delete collisionConfiguration;
                delete broadphase;
                delete dynamicsWorld;
            }
		};

        extern BulletPhysicsGlobalObjects bulletPhysicsGlobalObjects;


		#define MAX_SPOTLIGHTS 100
		#define MAX_DLIGHTS 20
		#define MAX_FRAMES 2

		extern bool  gToggleFullscreen;
		extern bool  gShowMeshes;
		extern bool  gShowRigidbodyMeshes;
		extern bool  gShowSkybox;
        extern bool  gDrawShadows;
		extern int   gHeight,
					 gWidth;
		extern bool  gIsScenePlaying;
		extern const int gmax_frames;
		extern VkSampleCountFlagBits gMSAAsamples;

		extern std::shared_ptr<spdlog::logger> gLogger;

		

		//extern VkRect2D   gEditor3DScissors;
		//extern VkViewport gEditor3DView;
		extern double DeltaTime;
	}
}

#endif