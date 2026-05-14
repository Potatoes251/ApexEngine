#ifndef SCRIPT_CAST_LUA_H
#define SCRIPT_CAST_LUA_H

#include <lua.hpp>

namespace Apex { class Application; }

namespace Apex::ScriptCast
{
    // Registers all cross-script communication globals into L.
    void Register(lua_State* L, Application* app);

    // Cast(objectName, scriptStem) - script instance table or nil
    // Mirrors UE5's Cast<T>(target) pattern.
    // objectName  : exact name of the Object in the scene hierarchy
    // scriptStem  : stem of the .lua file (e.g. "PlayerController")
    // Returns the live Lua instance table, or nil if not found.
    int Lua_Cast(lua_State* L);

    // GetScript(objectName) - script instance or nil
    // Returns the first ScriptComponent instance on the named object.
    int Lua_GetScript(lua_State* L);

    // FindObjectByName(name) - objectId or nil
    // Lets scripts store a lightweight id and re-resolve later.
    int Lua_FindObjectByName(lua_State* L);
}

#endif