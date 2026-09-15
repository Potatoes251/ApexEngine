#include "LuaFunction/CameraLua.h"
#include "LuaFunction/ComponentLua.h"

#include "Application.h"
#include "Scene.h"
#include "Camera.h"
#include "Object.h"
#include "LuaFunction/Vector3Lua.h"

void Apex::Camera::RegisterLua(Scripting::LuaManager& lua)
{
    lua.RegisterFunction("GetCamera_Internal", Apex::Camera::Lua_GetCamera);
    lua.RegisterFunction("Camera_SetMain_Internal", Apex::Camera::Lua_Camera_SetMain);
    lua.RegisterFunction("Camera_SetYaw_Internal", Apex::Camera::Lua_Camera_SetYaw);
    lua.RegisterFunction("Camera_SetPitch_Internal", Apex::Camera::Lua_Camera_SetPitch);
    lua.RegisterFunction("Camera_GetFront_Internal", Apex::Camera::Lua_Camera_GetForward);
    lua.RegisterFunction("Camera_GetRight_Internal", Apex::Camera::Lua_Camera_GetRight);
    lua.RegisterFunction("Camera_GetUp_Internal", Apex::Camera::Lua_Camera_GetUp);
}

int Apex::Camera::Lua_Camera_SetMain(lua_State* L)
{
    Rendering::Camera* camera = Comp::GetComponent<Rendering::Camera>(L, 1);

    if (!camera) return 0;

    camera->SetMainCamera(true);
    Rendering::Scene* scene = Application::Get()->GetScene();
    Rendering::Camera* mainCam = scene->GetMainCamera();
    if (mainCam == camera) return 0;

    if (!mainCam) mainCam->SetMainCamera(false);

    scene->SetMainCamera(camera);

    return 0;
}

int Apex::Camera::Lua_Camera_SetYaw(lua_State* L)
{
    Rendering::Camera* camera = Comp::GetComponent<Rendering::Camera>(L, 1);

    if (!camera) return 0;

    float yaw = lua_tonumber(L, 2);

    camera->SetYaw(yaw);

    return 0;
}

int Apex::Camera::Lua_Camera_SetPitch(lua_State* L)
{
    Rendering::Camera* camera = Comp::GetComponent<Rendering::Camera>(L, 1);

    if (!camera) return 0;

    float yaw = lua_tonumber(L, 2);

    camera->SetPitch(yaw);

    return 0;
}

int Apex::Camera::Lua_Camera_GetForward(lua_State* L)
{
	Rendering::Camera* camera = Comp::GetComponent<Rendering::Camera>(L, 1);

    if (!camera) return 0;

    Vector3::PushVector3(L, camera->GetFront());

	return 1;
}

int Apex::Camera::Lua_Camera_GetRight(lua_State* L)
{
    Rendering::Camera* camera = Comp::GetComponent<Rendering::Camera>(L, 1);

    if (!camera) return 0;

    Vector3::PushVector3(L, camera->GetRight());

    return 1;
}

int Apex::Camera::Lua_Camera_GetUp(lua_State* L)
{
    Rendering::Camera* camera = Comp::GetComponent<Rendering::Camera>(L, 1);

    if (!camera) return 0;

    Vector3::PushVector3(L, camera->GetUp());

    return 1;
}

int Apex::Camera::Lua_GetCamera(lua_State* L)
{
    lua_getfield(L, 1, "__object");
    auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!obj)
        return 0;

    Rendering::Camera* cam = obj->GetComponent<Rendering::Camera>();
    if (!cam)
        return 0;

    // push component as userdata
    lua_pushlightuserdata(L, cam);

    return 1;
}
