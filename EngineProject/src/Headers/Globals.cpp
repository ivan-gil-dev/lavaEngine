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
    namespace Lua {
        EngineAPI_GlobalVar lua_State* gLuaState;
    }
    namespace Globals {
        EngineAPI_GlobalVar KeyPressedEventHandler keyPressedEventHandler;
        EngineAPI_GlobalVar CursorPosition cursorPosition;
        EngineAPI_GlobalVar ShowCursorEventHandler showCursorEventHandler;

        EngineAPI_GlobalVar int   gHeight = 1366,
            gWidth = 768;

        EngineAPI_GlobalVar States_t states{
            false,//ToggleFullscreen
            true, //ShowMeshes
            true, //ShowRigidbodyMeshes
            true, //UseSceneCamera
            true, //ShowSkybox
            true  //DrawShadows
        };

        std::shared_ptr<spdlog::logger> gLogger = spdlog::basic_logger_mt("file", "logs/log.txt", true);

        EngineAPI_GlobalVar BulletPhysicsGlobalObjects bulletPhysicsGlobalObjects;
        EngineAPI_GlobalVar bool  gIsScenePlaying = false;
        EngineAPI_GlobalVar double  DeltaTime;

        const int gmax_frames = 2;

        EngineAPI_GlobalVar VkSampleCountFlagBits gMSAAsamples = VK_SAMPLE_COUNT_2_BIT;
        EngineAPI_GlobalVar lua_State* gLuaState;

        Application App;

        Scene* gScene;
    }
}