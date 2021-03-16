#include "Application.h"
#include "Camera.h"
#include "CommandPool.h"
#include "DescriptorPools.h"
#include "DescriptorSetLayouts.h"
#include "Device.h"
#include "Images.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Pipelines.h"
#include "Scene.h"
#include "Surface.h"
#include "SyncObjects.h"
namespace Engine {
	namespace Globals {
		DebugCamera debugCamera = {};
		CommandPool gCommandPool;
		DescriptorPoolForMesh				 gDescriptorPoolForMesh;
		DescriptorPoolForRigidBodyMesh	     gDescriptorPoolForRigidBodyMesh;
		DescriptorPoolForCubemapObjects		 gDescriptorPoolForCubemapObjects;
		DescriptorPoolForImgui				 gDescriptorPoolForImgui;
		DescriptorSetLayoutForMesh			 gSetLayoutForMesh;
		DescriptorSetLayoutForRigidBodyMesh  gSetLayoutForRigidBodyMesh;
		DescriptorSetLayoutForCubemapObjects gSetLayoutForCubemapObjects;
		Device gDevice;
		KeyPressedEventHandler keyPressedEventHandler;
		MouseMoveEventHandler mouseMoveEventHandler;
		ShowCursorEventHandler showCursorEventHandler;
		bool  gToggleFullscreen = false;
		bool  gShowMeshes = true;
		bool  gShowRigidbodyMeshes = true;
		bool  gShowSkybox = true;
		int   gHeight = 1366,
			  gWidth = 768;
		bool  gIsScenePlaying = false;
		const int gmax_frames = 2;
		VkSampleCountFlagBits gMSAAsamples = VK_SAMPLE_COUNT_4_BIT;
		btBroadphaseInterface* gBroadphase;
		btDefaultCollisionConfiguration* gCollisionConfiguration;
		btCollisionDispatcher* gDispatcher;
		btSequentialImpulseConstraintSolver* gSolver;
		btDynamicsWorld* gDynamicsWorld;
		VkRect2D   gEditor3DScissors = {};
		VkViewport gEditor3DView{};
		DepthImage gDepthImage;
		MultisamplingBuffer gMultisamplingBuffer;
		Instance gInstance;
		PhysicalDevice gPhysicalDevice;
		RenderPass gRenderPass;
		GraphicsPipelineForMesh			  gGraphicsPipelineForMesh;
		GraphicsPipelineForCubemapObjects gGraphicsPipelineForCubemapObjects;
		GraphicsPipelineForRigidBodyMesh  gGraphicsPipelineForRigidBodyMesh;
		Scene* gScene;
		Surface gSurface;
		Swapchain gSwapchain;
		SyncObjects gSyncObjects;
		Application App;
		double DeltaTime;

	}
}