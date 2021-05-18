#include "Application.h"
#include "Renderer/CommandPool.h"
#include "Renderer/DescriptorPools.h"
#include "Renderer/DescriptorSetLayouts.h"
#include "Renderer/Device.h"
#include "Renderer/Images.h"
#include "Renderer/VkInstance.h"
#include "Renderer/PhysicalDevice.h"
#include "Renderer/Pipelines.h"
#include "Scene.h"
#include "Renderer/Surface.h"
#include "Renderer/SyncObjects.h"
#include <spdlog/sinks/basic_file_sink.h>
namespace Engine {
	namespace Globals {

		KeyPressedEventHandler keyPressedEventHandler;
		CursorPosition cursorPosition;
		ShowCursorEventHandler showCursorEventHandler;
		bool  gDrawShadows = true;
		bool  gToggleFullscreen = false;
		bool  gShowMeshes = true;
		bool  gShowRigidbodyMeshes = true;
		bool  gShowSkybox = true;
		int   gHeight = 1366,
			  gWidth = 768;

		

		std::shared_ptr<spdlog::logger> gLogger = spdlog::basic_logger_mt("file", "logs/log.txt",true);

		EngineAPI_GlobalVar BulletPhysicsGlobalObjects bulletPhysicsGlobalObjects;
		EngineAPI_GlobalVar bool  gIsScenePlaying = false;
		EngineAPI_GlobalVar double  DeltaTime;

		const int gmax_frames = 2;

		VkSampleCountFlagBits gMSAAsamples = VK_SAMPLE_COUNT_2_BIT;

		Application App;

		

		Scene* gScene;
	}
}