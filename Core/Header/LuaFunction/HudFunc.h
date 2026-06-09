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

    // Generic
    int Lua_HUD_SetVisible(lua_State* L);
    int Lua_HUD_SetPosition(lua_State* L);
    int Lua_HUD_SetSize(lua_State* L);
    int Lua_HUD_GetVisible(lua_State* L);

    // UIText
    int Lua_HUD_SetText(lua_State* L);
    int Lua_HUD_SetTextColor(lua_State* L);
    int Lua_HUD_GetText(lua_State* L);

    // UIImage
    int Lua_HUD_SetTexture(lua_State* L);
    int Lua_HUD_SetImageTint(lua_State* L);

    // UIProgressBar
    int Lua_HUD_SetProgress(lua_State* L);
    int Lua_HUD_SetBarFillColor(lua_State* L);
    int Lua_HUD_SetBarBgColor(lua_State* L);
    int Lua_HUD_GetProgress(lua_State* L);

    // UIButton
    int Lua_HUD_SetButtonLabel(lua_State* L);
    int Lua_HUD_SetButtonColor(lua_State* L);
    int Lua_HUD_SetButtonTextColor(lua_State* L);
    int Lua_HUD_SetButtonHoverColor(lua_State* L);
    int Lua_HUD_SetButtonPressColor(lua_State* L);
    int Lua_HUD_SetButtonHAlign(lua_State* L);
    int Lua_HUD_SetButtonVAlign(lua_State* L);

    // UIPanel
    int Lua_HUD_SetPanelColor(lua_State* L);

    // UIButton
    int Lua_HUD_SetButtonCallBack(lua_State* L);
    int Lua_HUD_PressButton(lua_State* L);
}

#endif