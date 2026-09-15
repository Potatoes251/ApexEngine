#include "LuaFunction/HudFunc.h"
#include "Hud.h"

using namespace Apex::UserInterface;

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

        lua_register(L, "HUD_SetVisible", Lua_HUD_SetVisible);
        lua_register(L, "HUD_SetPosition", Lua_HUD_SetPosition);
        lua_register(L, "HUD_SetSize", Lua_HUD_SetSize);
        lua_register(L, "HUD_GetVisible", Lua_HUD_GetVisible);

        lua_register(L, "HUD_SetText", Lua_HUD_SetText);
        lua_register(L, "HUD_SetTextColor", Lua_HUD_SetTextColor);
        lua_register(L, "HUD_GetText", Lua_HUD_GetText);

        lua_register(L, "HUD_SetTexture", Lua_HUD_SetTexture);
        lua_register(L, "HUD_SetImageTint", Lua_HUD_SetImageTint);

        lua_register(L, "HUD_SetProgress", Lua_HUD_SetProgress);
        lua_register(L, "HUD_SetBarFillColor", Lua_HUD_SetBarFillColor);
        lua_register(L, "HUD_SetBarBgColor", Lua_HUD_SetBarBgColor);
        lua_register(L, "HUD_GetProgress", Lua_HUD_GetProgress);

        lua_register(L, "HUD_SetButtonLabel", Lua_HUD_SetButtonLabel);
        lua_register(L, "HUD_SetButtonColor", Lua_HUD_SetButtonColor);
        lua_register(L, "HUD_SetButtonTextColor", Lua_HUD_SetButtonTextColor);
        lua_register(L, "HUD_SetButtonHoverColor", Lua_HUD_SetButtonHoverColor);
        lua_register(L, "HUD_SetButtonPressColor", Lua_HUD_SetButtonPressColor);

        lua_register(L, "HUD_SetPanelColor", Lua_HUD_SetPanelColor);

        lua_register(L, "HUD_SetButtonCallBack", Lua_HUD_SetButtonCallBack);
        lua_register(L, "HUD_PressButton", Lua_HUD_PressButton);
    }

    static UIManager* GetUI(lua_State* L)
    {
        lua_pushstring(L, HUD_MANAGER_KEY);
        lua_gettable(L, LUA_REGISTRYINDEX);
        auto* ui = static_cast<Apex::UserInterface::UIManager*>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        return ui;
    }

    static UICanvas* GetCanvas(UIManager* ui, int handle)
    {
        if (!ui) return nullptr;
        HUDInstance* inst = ui->GetInstance(handle);
        return inst ? &inst->m_canvas : nullptr;
    }

    static UIElement* FindWidget(UICanvas& canvas, const std::string& name)
    {
        for (size_t i = 0; i < canvas.GetElementCount(); ++i)
        {
            UIElement* e = canvas.GetElement(i);
            if (!e) continue;
            if (e->m_name == name) return e;
            if (auto* panel = dynamic_cast<UIPanel*>(e))
                for (auto& child : panel->m_children)
                    if (child && child->m_name == name) return child.get();
        }
        return nullptr;
    }

    static UIColor ReadColor(lua_State* L, int rIdx)
    {
        return {
            static_cast<float>(luaL_checknumber(L, rIdx)),
            static_cast<float>(luaL_checknumber(L, rIdx + 1)),
            static_cast<float>(luaL_checknumber(L, rIdx + 2)),
            static_cast<float>(luaL_optnumber(L, rIdx + 3, 1.0))
        };
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

    int Lua_HUD_SetVisible(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        bool visibility = lua_toboolean(L, 3) != 0;

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (element) element->m_visible = visibility;
        return 0;
    }

    int Lua_HUD_SetPosition(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);

        float x = static_cast<float>(luaL_checknumber(L, 3));
        float y = static_cast<float>(luaL_checknumber(L, 4));

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (element) element->m_position = { x, y };
        return 0;
    }

    int Lua_HUD_SetSize(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        float width = static_cast<float>(luaL_checknumber(L, 3));
        float height = static_cast<float>(luaL_checknumber(L, 4));

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (element) element->m_size = { width, height };
        return 0;
    }

    int Lua_HUD_GetVisible(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) 
        { 
            lua_pushboolean(L, 0); 
            return 1; 
        }

        UIElement* element = FindWidget(*canvas, name);
        lua_pushboolean(L, element && element->m_visible ? 1 : 0);
        return 1;
    }

    int Lua_HUD_SetText(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        const char* text = luaL_checkstring(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* t = dynamic_cast<UIText*>(element))
            t->m_text = text;
        return 0;
    }

    int Lua_HUD_SetTextColor(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        UIColor color = ReadColor(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* text = dynamic_cast<UIText*>(element))
            text->m_textColor = color;
        return 0;
    }

    int Lua_HUD_GetText(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) 
        { 
            lua_pushstring(L, ""); 
            return 1; 
        }

        UIElement* element = FindWidget(*canvas, name);
        if (auto* text = dynamic_cast<UIText*>(element))
            lua_pushstring(L, text->m_text.c_str());
        else
            lua_pushstring(L, "");
        return 1;
    }

    int Lua_HUD_SetTexture(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        const char* path = luaL_checkstring(L, 3);

        auto* ui = GetUI(L);
        if (!ui) return 0;

        auto* canvas = GetCanvas(ui, handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* img = dynamic_cast<UIImage*>(element))
        {
            img->m_texturePath = path;
            img->m_glTexID = 0;
            img->LoadTexture(*ui->GetResourceManager());
        }
        return 0;
    }

    int Lua_HUD_SetImageTint(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        UIColor color = ReadColor(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (dynamic_cast<UIImage*>(element))
            element->m_color = color;
        return 0;
    }

    int Lua_HUD_SetProgress(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);

        float val = static_cast<float>(luaL_checknumber(L, 3));
        if (val < 0.f) val = 0.f;
        if (val > 1.f) val = 1.f;

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* progressBar = dynamic_cast<UIProgressBar*>(element))
            progressBar->m_value = val;
        return 0;
    }

    int Lua_HUD_SetBarFillColor(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        UIColor color = ReadColor(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* progressBar = dynamic_cast<UIProgressBar*>(element))
            progressBar->m_colorFill = color;
        return 0;
    }

    int Lua_HUD_SetBarBgColor(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        UIColor color = ReadColor(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* progressBar = dynamic_cast<UIProgressBar*>(element))
            progressBar->m_colorBg = color;
        return 0;
    }

    int Lua_HUD_GetProgress(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) 
        { 
            lua_pushnumber(L, 0.0); 
            return 1; 
        }

        UIElement* element = FindWidget(*canvas, name);
        if (auto* progressBar = dynamic_cast<UIProgressBar*>(element))
            lua_pushnumber(L, static_cast<double>(progressBar->m_value));
        else
            lua_pushnumber(L, 0.0);
        return 1;
    }

    int Lua_HUD_SetButtonLabel(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        const char* label = luaL_checkstring(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* button = dynamic_cast<UIButton*>(element))
            button->m_labelText.m_text = label;
        return 0;
    }

    int Lua_HUD_SetButtonColor(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        UIColor color = ReadColor(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* button = dynamic_cast<UIButton*>(element))
            button->m_color = color;
        return 0;
    }

    int Lua_HUD_SetButtonTextColor(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        UIColor color = ReadColor(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* button = dynamic_cast<UIButton*>(element))
            button->m_labelText.m_textColor = color;
        return 0;
    }

    int Lua_HUD_SetButtonHoverColor(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        UIColor color = ReadColor(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* button = dynamic_cast<UIButton*>(element))
            button->m_colorHover = color;
        return 0;
    }

    int Lua_HUD_SetButtonPressColor(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        UIColor color = ReadColor(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* button = dynamic_cast<UIButton*>(element))
            button->m_colorPressed = color;
        return 0;
    }

    int Lua_HUD_SetButtonHAlign(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        int align = static_cast<int>(luaL_checkinteger(L, 3));

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* button = dynamic_cast<UIButton*>(element))
            button->m_labelText.m_hAlign = static_cast<TextHAlign>(align);
        return 0;
    }

    int Lua_HUD_SetButtonVAlign(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        int align = static_cast<int>(luaL_checkinteger(L, 3));

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* button = dynamic_cast<UIButton*>(element))
            button->m_labelText.m_vAlign = static_cast<TextVAlign>(align);
        return 0;
    }

    int Lua_HUD_SetPanelColor(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);
        UIColor color = ReadColor(L, 3);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* panel = dynamic_cast<UIPanel*>(element))
            panel->m_colorBg = color;
        return 0;
    }

    int Lua_HUD_SetButtonCallBack(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* button = dynamic_cast<UIButton*>(element))
        {
            button->m_luaState = L;

            //func ref
            lua_pushvalue(L, 3);
            button->m_luaFuncRef = luaL_ref(L, LUA_REGISTRYINDEX);

            // wrap into std::function
            button->m_onClick = [button]()
                {
                    lua_State* L = button->m_luaState;

                    lua_rawgeti(L, LUA_REGISTRYINDEX, button->m_luaFuncRef);

                    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
                    {
                        const char* err = lua_tostring(L, -1);
                        lua_pop(L, 1);
                    }
                };
        }

        return 0;
    }

    int Lua_HUD_PressButton(lua_State* L)
    {
        int handle = static_cast<int>(luaL_checkinteger(L, 1));
        const char* name = luaL_checkstring(L, 2);

        auto* canvas = GetCanvas(GetUI(L), handle);
        if (!canvas) return 0;

        UIElement* element = FindWidget(*canvas, name);
        if (auto* button = dynamic_cast<UIButton*>(element))
        {
            if (button->m_onClick)
                button->m_onClick();
        }

        return 0;
    }

} // namespace Apex::HudBindings