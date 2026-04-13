#ifndef HUD_FUNC_H
#define HUD_FUNC_H

// ============================================================
// HudFunc.h  —  Lua bindings for the HUD system.
//
// API exposed to Lua:
//
//   handle = CreateHUD("GameHUD")   -- load Assets/***/GameHUD.hud
//   ShowHUD(handle)                 -- make it visible
//   HideHUD(handle)                 -- hide it
//   DestroyHUD(handle)              -- free the instance
//   visible = IsHUDVisible(handle)  -- bool query
//
// Example script:
//   local hud = 0
//   function self:Start()
//       hud = CreateHUD("GameHUD")
//       ShowHUD(hud)
//   end
//   function self:Update(dt)
//       if condition then HideHUD(hud) end
//   end
// ============================================================

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