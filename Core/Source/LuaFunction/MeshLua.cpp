#include "LuaFunction/MeshLua.h"
#include "LuaFunction/ObjectLua.h"

#include "MeshRenderer.h"
#include "Object.h"

#include "Log.h"

using namespace Apex::Data;
using namespace Apex::Rendering;

void Apex::Mezh::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetMesh_Internal", Apex::Mezh::Lua_GetMesh);
    lua.RegisterFunction("SetUniform_Internal", Apex::Mezh::Lua_SetUniform);
}

MeshRenderer* Apex::Mezh::GetMesh(lua_State* L, int index)
{
    lua_getfield(L, index, "__object");
    MeshRenderer* mesh = static_cast<MeshRenderer*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return mesh;
}

int Apex::Mezh::Lua_SetUniform(lua_State* L)
{
    MeshRenderer* mesh = GetMesh(L, 1);

    if (!mesh)
    {
        LOG_ERROR_CAT("Scripting", "couldnt get the mesh in MeshRenderer::SetUniform()");
        return 0;
    }

    Resources::ResourceHandle<Material> const& mat = mesh->GetMaterial();

    if (!mat.IsValid())
    {
        LOG_WARNING_CAT("Scripting", "the material is invalid in SetUniform()");
        return 0;
    }

    const char* uniformName = luaL_checkstring(L, 2);

    if (!uniformName)
    {
        LOG_ERROR_CAT("Scripting", "SetUniform() requires a str");
        return 0;
    }

    mat->SetMaterialUniform(uniformName, (float)lua_tonumber(L, 3));

    return 0;
}

int Apex::Mezh::Lua_GetMesh(lua_State* L)
{
    Object* obj = GetObject(L, 1);
    if (!obj)
        return 0;

    MeshRenderer* mesh = obj->GetComponent<MeshRenderer>();
    if (!mesh)
        return 0;

    // push component as userdata
    lua_pushlightuserdata(L, mesh);

    return 1;
}