#include "LuaFunction/HierarchyFunc.h"
#include "SceneGraph.h"

using namespace Apex::SceneGraph;

Apex::Data::Object* Apex::Hierarchy::GetObject(lua_State* L, int index)
{
    lua_getfield(L, index, "__object");
    auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return obj;
}

int Apex::Hierarchy::Lua_AddChild(lua_State* L)
{
    auto* parent = GetObject(L, 1);
    auto* child = GetObject(L, 2);

    if (!parent || !child)
        return 0;

    parent->GetSceneNode()->AddChild(child->GetSceneNode());

    return 0;
}

int Apex::Hierarchy::Lua_RemoveChild(lua_State* L)
{
    auto* parent = GetObject(L, 1);
    auto* child = GetObject(L, 2);

    if (!parent || !child)
        return 0;

    parent->GetSceneNode()->DetachChild(child->GetSceneNodeRaw());

    return 0;
}

int Apex::Hierarchy::Lua_SetParent(lua_State* L)
{
    auto* childObj = GetObject(L, 1);
    auto* newParentObj = GetObject(L, 2);

    if (!childObj || !newParentObj)
        return 0;

    std::shared_ptr<SceneNode> const& childNode = childObj->GetSceneNode();
    std::shared_ptr<SceneNode> const& parentNode = newParentObj->GetSceneNode();

    if (!childNode || !parentNode)
        return 0;

    childNode->SetParent(parentNode);

    return 0;
}