#pragma once
#include "Globals.h"
namespace Engine {
    namespace Lua {
        class Script
        {
            bool CheckLua(int r) {
                if (r != LUA_OK)
                {
                    std::string err = lua_tostring(gLuaState, -1);
                    spdlog::warn("Lua error: {0}", err);
                    std::cout << err << std::endl;
                }
                return r;
            }

        public:
            bool doFile(std::string path) {
                bool r = CheckLua(luaL_dofile(gLuaState, path.c_str()));
                return r;
            }

            template <typename T> T getVar(std::string varName) {
            }

            template <> int getVar<int>(std::string varName) {
                lua_getglobal(Engine::Lua::gLuaState, varName.c_str());
                return lua_tointeger(Engine::Lua::gLuaState, -1);
            }

            template <> float getVar<float>(std::string varName) {
                lua_getglobal(Engine::Lua::gLuaState, varName.c_str());
                return lua_tonumber(Engine::Lua::gLuaState, -1);
            }

            template <> bool getVar<bool>(std::string varName) {
                lua_getglobal(Engine::Lua::gLuaState, varName.c_str());
                return lua_toboolean(Engine::Lua::gLuaState, -1);
            }

            template <> std::string getVar<std::string>(std::string varName) {
                lua_getglobal(Engine::Lua::gLuaState, varName.c_str());
                return lua_tostring(Engine::Lua::gLuaState, -1);
            }
        };
    }
}
