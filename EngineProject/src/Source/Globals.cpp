#include "../Headers/Application.h"
#include "../Headers/Renderer/CommandPool.h"
#include "../Headers/Renderer/DescriptorPools.h"
#include "../Headers/Renderer/DescriptorSetLayouts.h"
#include "../Headers/Renderer/Device.h"
#include "../Headers/Renderer/Images.h"
#include "../Headers/Renderer/VkInstance.h"
#include "../Headers/Renderer/PhysicalDevice.h"
#include "../Headers/Renderer/Pipelines.h"
#include "../Headers/Scene.h"
#include "../Headers/Renderer/Surface.h"
#include "../Headers/Renderer/SyncObjects.h"
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

        EngineAPI_GlobalVar VkSampleCountFlagBits gMSAAsamples = VK_SAMPLE_COUNT_4_BIT;
        EngineAPI_GlobalVar lua_State* gLuaState;
        EngineAPI_GlobalVar bool enableEditor;

        Application App;

        Scene* gScene;
    }
}