#include "LuaFunction/HudFunc.h"
#include "Hud.h"

static const char* HUD_MANAGER_KEY = "__apex_ui_manager";

namespace Apex::HudBindings
{
    void RegisterHudFunctions(lua_State* L, Apex::UserInterface::UIManager* ui)
    {
        if (!L || !ui) return;

        // Store UIManager* in the Lua registry
        lua_pushstring(L, HUD_MANAGER_KEY);
        lua_pushlightuserdata(L, static_cast<void*>(ui));
        lua_settable(L, LUA_REGISTRYINDEX);

        lua_register(L, "CreateHUD", Lua_CreateHUD);
        lua_register(L, "ShowHUD", Lua_ShowHUD);
        lua_register(L, "HideHUD", Lua_HideHUD);
        lua_register(L, "DestroyHUD", Lua_DestroyHUD);
        lua_register(L, "IsHUDVisible", Lua_IsHUDVisible);
    }

    static Apex::UserInterface::UIManager* GetUI(lua_State* L)
    {
        lua_pushstring(L, HUD_MANAGER_KEY);
        lua_gettable(L, LUA_REGISTRYINDEX);
        auto* ui = static_cast<Apex::UserInterface::UIManager*>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        return ui;
    }

    int Lua_CreateHUD(lua_State* L)
    {
        const char* name = luaL_checkstring(L, 1);
        auto* ui = GetUI(L);
        int handle = ui ? ui->CreateHUD(name) : 0;
        lua_pushinteger(L, handle);
        return 1;
    }

    int Lua_ShowHUD(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        auto* ui = GetUI(L);
        if (ui) ui->ShowHUD(handle);
        return 0;
    }

    int Lua_HideHUD(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        auto* ui = GetUI(L);
        if (ui) ui->HideHUD(handle);
        return 0;
    }

    int Lua_DestroyHUD(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        auto* ui = GetUI(L);
        if (ui) ui->DestroyHUD(handle);
        return 0;
    }

    int Lua_IsHUDVisible(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        auto* ui = GetUI(L);
        lua_pushboolean(L, ui ? (ui->IsHUDVisible(handle) ? 1 : 0) : 0);
        return 1;
    }

} // namespace Apex::HudBindings