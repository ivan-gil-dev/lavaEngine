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
namespace Engine{

	

	class __declspec(dllexport) Scene {
		std::vector<DataTypes::DirectionalLightAttributes_t*> directionalLightAttributes;

		std::vector<DataTypes::PointLightAttributes_t*> pointLightAttributes;
		std::vector<Entity*> entities;

		nlohmann::ordered_json sceneJson;

		std::string scenePath;

	public:

		std::string GetScenePath();

		void Load(std::string path);

		void New();

		void Save();

		void SaveAs(std::string path);

		std::vector<Entity*>* pGetVectorOfEntities();

		Scene();

		std::vector<DataTypes::DirectionalLightAttributes_t*>* pGetVectorOfDirectionalLightAttributes();

		std::vector<DataTypes::PointLightAttributes_t*>* pGetVectorOfSpotlightAttributes();

		void CleanScene();
	};

	namespace Globals{
		extern Scene* gScene;
	}
}

#endif
#  pragma warning( pop )