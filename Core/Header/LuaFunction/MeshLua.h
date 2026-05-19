#ifndef MESH_LUA
#define MESH_LUA

#include <lua.hpp>

#include "LuaManager.h"

namespace Apex::Rendering { class MeshRenderer; }

// Mesh is a class so the namespace must be different

namespace Apex::Mezh
{
	void RegisterLua(Scripting::LuaManager& lua);

	int Lua_SetUniform(lua_State* L);

	int Lua_GetMesh(lua_State* L);
}

#endif // !MESH_LUA