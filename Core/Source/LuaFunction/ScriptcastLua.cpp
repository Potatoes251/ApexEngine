#include "LuaFunction/ScriptcastLua.h"

#include "Application.h"
#include "Object.h"
#include "ScriptComponent.h"

#include "Log.h"

#include <filesystem>

using namespace Apex;
using namespace Apex::Rendering;
using namespace Apex::Scripting;

static const char* APP_KEY = "__apex_app";

namespace Apex::ScriptCast
{
    void Register(lua_State* L, Application* app)
    {
        if (!L || !app) return;

        lua_pushstring(L, APP_KEY);
        lua_pushlightuserdata(L, static_cast<void*>(app));
        lua_settable(L, LUA_REGISTRYINDEX);

        lua_register(L, "Cast", Lua_Cast);
        lua_register(L, "GetScript", Lua_GetScript);
        lua_register(L, "FindObjectByName", Lua_FindObjectByName);
    }

    static Scene* GetScene(lua_State* L)
    {
        lua_pushstring(L, APP_KEY);
        lua_gettable(L, LUA_REGISTRYINDEX);

        auto* app = static_cast<Application*>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        return app ? app->GetScene() : nullptr;
    }

    // Find a ScriptComponent whose script stem matches scriptStem.
    // If scriptStem is empty, returns the first ScriptComponent.
    static ScriptComponent* FindScript(Data::Object* obj, const std::string& scriptStem)
    {
        for (ScriptComponent* scriptComponent : obj->GetComponentsByType<ScriptComponent>())
        {
            if (scriptStem.empty()) return scriptComponent;

            std::string stem = std::filesystem::path(scriptComponent->GetScriptPath()).stem().string();
            if (stem == scriptStem) return scriptComponent;
        }
        return nullptr;
    }

    // Push the Lua instance table for a ScriptComponent onto the stack.
    // Returns true on success, false (and pushes nil) on failure.
    static bool PushInstance(lua_State* L, ScriptComponent* scriptComponent)
    {
        if (!scriptComponent) 
        { 
            lua_pushnil(L); 
            return false; 
        }

        int ref = scriptComponent->GetInstanceRef();
        if (ref == LUA_REFNIL || ref == LUA_NOREF) 
        { 
            lua_pushnil(L); 
            return false; 
        }

        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        if (!lua_istable(L, -1)) 
        { 
            lua_pop(L, 1); 
            lua_pushnil(L); 
            return false; 
        }
        return true;
    }

    // Cast(objectName, scriptStem) - instance table or nil
    int Lua_Cast(lua_State* L)
    {
        const char* objName = luaL_checkstring(L, 1);
        const char* scriptStem = luaL_optstring(L, 2, "");

        Scene* scene = GetScene(L);
        if (!scene) 
        { 
            lua_pushnil(L); 
            return 1; 
        }

        for (const auto& obj : scene->GetObjects())
        {
            if (obj->GetName() != objName) continue;
            ScriptComponent* scriptComponent = FindScript(obj.get(), scriptStem);
            if (!PushInstance(L, scriptComponent))
            {
                lua_pushnil(L);
            }
            return 1;
        }

        lua_pushnil(L);
        return 1;
    }

    // GetScript(objectName) - first script instance or nil
    int Lua_GetScript(lua_State* L)
    {
        const char* objName = luaL_checkstring(L, 1);
        Scene* scene = GetScene(L);
        if (!scene) 
        { 
            lua_pushnil(L); 
            return 1; 
        }

        for (const auto& obj : scene->GetObjects())
        {
            if (obj->GetName() != objName) continue;
            ScriptComponent* scriptComponent = obj->GetComponent<ScriptComponent>();
            if (!PushInstance(L, scriptComponent))
            {
                lua_pushnil(L);
            }
            return 1;
        }

        lua_pushnil(L);
        return 1;
    }

    // FindObjectByName(name) - objectId (integer) or nil
    int Lua_FindObjectByName(lua_State* L)
    {
        const char* name = luaL_checkstring(L, 1);
        Scene* scene = GetScene(L);
        if (!scene) 
        { 
            lua_pushnil(L); 
            return 1; 
        }

        for (const auto& obj : scene->GetObjects())
        {
            if (obj->GetName() == name)
            {
                lua_pushinteger(L, static_cast<lua_Integer>(obj->GetId()));
                return 1;
            }
        }

        lua_pushnil(L);
        return 1;
    }

} // namespace Apex::ScriptCast