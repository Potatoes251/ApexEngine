#include "LuaFunction/InputLua.h"

using namespace Apex::Input;

void Apex::Input::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetAxis_Internal", Apex::Input::Lua_GetAxis);
    lua.RegisterFunction("IsKeyDown_Internal", Apex::Input::Lua_IsKeyDown);
}

Key Apex::Input::StringToKey(char const* keyStr)
{
    if (strcmp(keyStr, "W") == 0) return Key::W;
    if (strcmp(keyStr, "A") == 0) return Key::A;
    if (strcmp(keyStr, "S") == 0) return Key::S;
    if (strcmp(keyStr, "D") == 0) return Key::D;
    if (strcmp(keyStr, "Q") == 0) return Key::Q;
    if (strcmp(keyStr, "E") == 0) return Key::E;
    if (strcmp(keyStr, "P") == 0) return Key::P;
    if (strcmp(keyStr, "F") == 0) return Key::F;

    if (strcmp(keyStr, "Up") == 0) return Key::Up;
    if (strcmp(keyStr, "Down") == 0) return Key::Down;
    if (strcmp(keyStr, "Left") == 0) return Key::Left;
    if (strcmp(keyStr, "Right") == 0) return Key::Right;

    if (strcmp(keyStr, "Escape") == 0) return Key::Escape;
    if (strcmp(keyStr, "Space") == 0) return Key::Space;

    return Key::Unknown;
}

int Apex::Input::Lua_GetAxis(lua_State* L)
{
    const char* axisStr = luaL_checkstring(L, 1);

    if (strcmp(axisStr, "Horizontal") == 0)
    {
        float value = 0.f;
        if (InputSystem::Get().IsKeyDown(Key::A))
            value += 1.f;
        if (InputSystem::Get().IsKeyDown(Key::D))
            value -= 1.f;

        lua_pushnumber(L, value);
    }
    if (strcmp(axisStr, "Vertical") == 0)
    {
        float value = 0.f;
        if (InputSystem::Get().IsKeyDown(Key::W))
            value += 1.f;
        if (InputSystem::Get().IsKeyDown(Key::S))
            value -= 1.f;
        lua_pushnumber(L, value);
    }

    return 1;
}

int Apex::Input::Lua_IsKeyDown(lua_State* L)
{
    const char* keyStr = luaL_checkstring(L, 1);

    lua_pushboolean(L, InputSystem::Get().IsKeyDown(StringToKey(keyStr)));

    return 1;
}
