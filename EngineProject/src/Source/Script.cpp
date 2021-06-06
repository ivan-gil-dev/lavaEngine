#include "../Headers/Script.h"
#include <LuaBridge/detail/LuaRef.h>
bool Engine::Lua::Script::CheckLua(int r)
{
    if (r != LUA_OK)
    {
        std::string err = lua_tostring(gLuaState, -1);
        spdlog::warn("Lua error: {0}", err);
        std::cout << err << std::endl;
        path = "";
    }
    return r;
}

int Engine::Lua::Script::doScriptUpdate(Entity* entity)
{
    //Компилировать скрипт
    int r = doScript();
    //Если нет ошибок то выполнить функцию Update
    if (r == LUA_OK)
    {
        luabridge::LuaRef update = luabridge::getGlobal(gLuaState, "update");
        update(entity);
        return 0;
    }
    else {
        return -1;
    }
}

int Engine::Lua::Script::doScript()
{
    if (path != "")
    {
        int r = CheckLua(luaL_dofile(gLuaState, path.c_str()));
        return r;
    }
    else {
        return -1;
    }
}

void Engine::Lua::Script::SetScriptPath(std::string scrPath)
{
    path = scrPath;
}

template <typename T> T Engine::Lua::Script::getVar(std::string varName) {
    return 0;
}

std::string Engine::Lua::Script::GetScriptPath()
{
    return path;
}

template <> lua_Integer Engine::Lua::Script::getVar<lua_Integer>(std::string varName) {
    if (path != "")
    {
        lua_getglobal(Engine::Lua::gLuaState, varName.c_str());
        return lua_tointeger(Engine::Lua::gLuaState, -1);
    }
    return 0;
}

template <> lua_Number Engine::Lua::Script::getVar<lua_Number>(std::string varName) {
    if (path != "")
    {
        lua_getglobal(Engine::Lua::gLuaState, varName.c_str());
        return lua_tonumber(Engine::Lua::gLuaState, -1);
    }
    return 0;
}

template <> bool Engine::Lua::Script::getVar<bool>(std::string varName) {
    if (path != "")
    {
        lua_getglobal(Engine::Lua::gLuaState, varName.c_str());
        bool var = lua_toboolean(Engine::Lua::gLuaState, -1);
        return var;
    }
    return 0;
}

template <> std::string Engine::Lua::Script::getVar<std::string>(std::string varName) {
    if (path != "")
    {
        lua_getglobal(Engine::Lua::gLuaState, varName.c_str());
        std::string var = lua_tostring(Engine::Lua::gLuaState, -1);
        return var;
    }
    return "";
}