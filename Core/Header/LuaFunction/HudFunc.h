#ifndef HUD_FUNC_H
#define HUD_FUNC_H

#include <lua.hpp>

namespace Apex::UserInterface { class UIManager; }

namespace Apex::HudBindings
{
    // Call once after UIManager is created.
    void RegisterHudFunctions(lua_State* L, Apex::UserInterface::UIManager* ui);

    int Lua_CreateHUD(lua_State* L);
    int Lua_ShowHUD(lua_State* L);
    int Lua_HideHUD(lua_State* L);
    int Lua_DestroyHUD(lua_State* L);
    int Lua_IsHUDVisible(lua_State* L);
}

#endif