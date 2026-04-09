#include "LuaFunction/InputLua.h"

using namespace Apex::Input;

void Apex::Input::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetAxis_Internal", Apex::Input::Lua_GetAxis);
    lua.RegisterFunction("IsKeyDown_Internal", Apex::Input::Lua_IsKeyDown);
}

Key Apex::Input::StringToKey(char const* keyStr)
{
    if (strcmp(keyStr, "A") == 0) return Key::A;
    if (strcmp(keyStr, "B") == 0) return Key::B;
    if (strcmp(keyStr, "C") == 0) return Key::C;
    if (strcmp(keyStr, "D") == 0) return Key::D;
    if (strcmp(keyStr, "E") == 0) return Key::E;
    if (strcmp(keyStr, "F") == 0) return Key::F;
    if (strcmp(keyStr, "G") == 0) return Key::G;
    if (strcmp(keyStr, "H") == 0) return Key::H;
    if (strcmp(keyStr, "I") == 0) return Key::I;
    if (strcmp(keyStr, "J") == 0) return Key::J;
    if (strcmp(keyStr, "K") == 0) return Key::K;
    if (strcmp(keyStr, "L") == 0) return Key::L;
    if (strcmp(keyStr, "M") == 0) return Key::M;
    if (strcmp(keyStr, "N") == 0) return Key::N;
    if (strcmp(keyStr, "O") == 0) return Key::O;
    if (strcmp(keyStr, "P") == 0) return Key::P;
    if (strcmp(keyStr, "Q") == 0) return Key::Q;
    if (strcmp(keyStr, "R") == 0) return Key::R;
    if (strcmp(keyStr, "S") == 0) return Key::S;
    if (strcmp(keyStr, "T") == 0) return Key::T;
    if (strcmp(keyStr, "U") == 0) return Key::U;
    if (strcmp(keyStr, "V") == 0) return Key::V;
    if (strcmp(keyStr, "W") == 0) return Key::W;
    if (strcmp(keyStr, "X") == 0) return Key::X;
    if (strcmp(keyStr, "Y") == 0) return Key::Y;
    if (strcmp(keyStr, "Z") == 0) return Key::Z;

    if (strcmp(keyStr, "0") == 0) return Key::N0;
    if (strcmp(keyStr, "1") == 0) return Key::N1;
    if (strcmp(keyStr, "2") == 0) return Key::N2;
    if (strcmp(keyStr, "3") == 0) return Key::N3;
    if (strcmp(keyStr, "4") == 0) return Key::N4;
    if (strcmp(keyStr, "5") == 0) return Key::N5;
    if (strcmp(keyStr, "6") == 0) return Key::N6;
    if (strcmp(keyStr, "7") == 0) return Key::N7;
    if (strcmp(keyStr, "8") == 0) return Key::N8;
    if (strcmp(keyStr, "9") == 0) return Key::N9;

    if (strcmp(keyStr, "Up") == 0)      return Key::Up;
    if (strcmp(keyStr, "Down") == 0)    return Key::Down;
    if (strcmp(keyStr, "Left") == 0)    return Key::Left;
    if (strcmp(keyStr, "Right") == 0)   return Key::Right;

    if (strcmp(keyStr, "Tab") == 0)         return Key::Tab;
    if (strcmp(keyStr, "Ctrl") == 0)        return Key::Ctrl;
    if (strcmp(keyStr, "Space") == 0)       return Key::Space;
    if (strcmp(keyStr, "Enter") == 0)       return Key::Enter;
    if (strcmp(keyStr, "Shift") == 0)       return Key::Shift;
    if (strcmp(keyStr, "Escape") == 0)      return Key::Escape;
    if (strcmp(keyStr, "BackSpace") == 0)   return Key::BackSpace;

    return Key::Unknown;
}

int Apex::Input::Lua_GetAxis(lua_State* L)
{
    const char* axisStr = luaL_checkstring(L, 1);

    if (strcmp(axisStr, "Horizontal") == 0)
    {
        float value = 0.f;
        if (InputSystem::Get().IsKeyDown(Key::A))
            value -= 1.f;
        if (InputSystem::Get().IsKeyDown(Key::D))
            value += 1.f;

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
