#pragma once
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#include "Globals.h"
namespace Engine {
    class Entity;
    class GameObject;

    namespace Lua {
        class EngineAPI_Export Script
        {
            bool CheckLua(int r);

            std::string path = "";

        public:
            int doScriptUpdate(Entity* entity);

            int doScriptOnCollision(GameObject* obj1, GameObject* obj2);

            int doScript();

            void SetScriptPath(std::string scrPath);

            std::string GetScriptPath();

            template <typename T> T getVar(std::string varName);

            template <> lua_Integer getVar<lua_Integer>(std::string varName);

            template <> lua_Number getVar<lua_Number>(std::string varName);

            template <> bool getVar<bool>(std::string varName);

            template <> std::string getVar<std::string>(std::string varName);
        };
    }
}
# pragma warning( pop )