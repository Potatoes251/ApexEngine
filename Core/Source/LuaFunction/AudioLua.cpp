#include "LuaFunction/AudioLua.h"

#include "LuaFunction/Vector3Lua.h"

#include "Object.h"

#include "Audio.h"
#include "AudioComponent.h"

void Apex::Audio::RegisterLua(Scripting::LuaManager& lua)
{
	lua.RegisterFunction("GetAudioComp_Internal", Apex::Audio::Lua_GetAudioComponent);
	lua.RegisterFunction("AudioComp_Play_Internal", Apex::Audio::Lua_AudioComp_Play);
	lua.RegisterFunction("AudioComp_Play3D_Internal", Apex::Audio::Lua_AudioComp_Play3D);
	lua.RegisterFunction("AudioComp_PlayAt_Internal", Apex::Audio::Lua_AudioComp_PlayAt);
}

Apex::Audio::AudioComponent* Apex::Audio::GetAudioComponent(lua_State* L, int index)
{
    lua_getfield(L, index, "__object");
    AudioComponent* audio = static_cast<AudioComponent*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return audio;
}

int Apex::Audio::Lua_AudioComp_Play(lua_State* L)
{
    AudioComponent* comp = GetAudioComponent(L, 1);

    if (comp) comp->Play();

    return 0;
}

int Apex::Audio::Lua_AudioComp_Play3D(lua_State* L)
{
    AudioComponent* comp = GetAudioComponent(L, 1);

    if (comp) comp->Play3D();

    return 0;
}

int Apex::Audio::Lua_AudioComp_PlayAt(lua_State* L)
{
    AudioComponent* comp = GetAudioComponent(L, 1);

    Vector3::LuaVector3* dir = Vector3::CheckVector3(L, 2);

    if (comp) comp->PlayAt(dir->m_value);

    return 0;
}

int Apex::Audio::Lua_GetAudioComponent(lua_State* L)
{
    lua_getfield(L, 1, "__object");
    auto* obj = static_cast<Apex::Data::Object*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!obj)
        return 0;

    AudioComponent* audioComp = obj->GetComponent<AudioComponent>();
    if (!audioComp)
        return 0;

    // push component as userdata
    lua_pushlightuserdata(L, audioComp);

    return 1;
}
