#  pragma warning( push )
#  pragma warning( disable: 4251 )
#ifndef scene_h
#define scene_h

#include "Globals.h"
#include "Renderer/DataTypes.h"
#include <vector>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include "Entities.h"
namespace Engine {
    class EngineAPI_Export Scene {
        std::vector<DataTypes::DirectionalLightAttributes_t*> directionalLightAttributes;//Свойства направленных источников света

        std::vector<DataTypes::PointLightAttributes_t*> pointLightAttributes;//Свойства точечных источников света

        std::vector<Entity*> entities;//Объекты в сцене

        nlohmann::ordered_json sceneJson;//JSON парсер

        std::string scenePath;//Путь к сцене

        std::vector<Camera*> cameras;

        Camera* activeCamera;

    public:

        std::string GetScenePath();

        void Load(const std::string& path);

        void New();

        void Save();

        void SaveAs(const std::string& path);

        void Load_FromThread(std::string path, std::atomic<bool>& ready);

        void New_FromThread(std::atomic<bool>& ready);

        void Save_FromThread(std::atomic<bool>& ready);

        void SaveAs_FromThread(std::string path, std::atomic<bool>& ready);

        std::vector<Entity*>* pGetVectorOfEntities();

        Scene();

        std::vector<DataTypes::DirectionalLightAttributes_t*>* pGetVectorOfDirectionalLightAttributes();

        std::vector<DataTypes::PointLightAttributes_t*>* pGetVectorOfSpotlightAttributes();

        std::vector<Camera*>* pGetVectorOfCameras();

        void SetActiveCameraFromIndex(int id);

        Camera* pGetActiveCamera();

        Camera GetActiveCamera();

        void UpdateActiveCamera();

        void CleanScene();
    };

    namespace Globals {
        extern Scene* gScene;
    }
}

#endif
#  pragma warning( pop )