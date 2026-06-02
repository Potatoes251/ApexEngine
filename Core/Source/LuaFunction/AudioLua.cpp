#include "LuaFunction/AudioLua.h"
#include "LuaFunction/ComponentLua.h"

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
	lua.RegisterFunction("AudioComp_Stop_Internal", Apex::Audio::Lua_AudioComp_Stop);
	lua.RegisterFunction("AudioComp_SetSound_Internal", Apex::Audio::Lua_AudioComp_SetSound);
	lua.RegisterFunction("AudioComp_SetVolume_Internal", Apex::Audio::Lua_AudioComp_SetVolume);
	lua.RegisterFunction("AudioComp_SetChannel_Internal", Apex::Audio::Lua_AudioComp_SetChannel);
	lua.RegisterFunction("AudioComp_SetLoop_Internal", Apex::Audio::Lua_AudioComp_SetLoop);
}

int Apex::Audio::Lua_AudioComp_Play(lua_State* L)
{
    AudioComponent* comp = Comp::GetComponent<AudioComponent>(L, 1);

    if (comp) comp->Play();

    return 0;
}

int Apex::Audio::Lua_AudioComp_Play3D(lua_State* L)
{
    AudioComponent* comp = Comp::GetComponent<AudioComponent>(L, 1);

    if (comp) comp->Play3D();

    return 0;
}

int Apex::Audio::Lua_AudioComp_PlayAt(lua_State* L)
{
    AudioComponent* comp = Comp::GetComponent<AudioComponent>(L, 1);

    Vector3::LuaVector3* dir = Vector3::CheckVector3(L, 2);

    if (comp) comp->PlayAt(dir->m_value);

    return 0;
}

int Apex::Audio::Lua_AudioComp_Stop(lua_State* L)
{
	AudioComponent* comp = Comp::GetComponent<AudioComponent>(L, 1);

	if (comp) comp->Stop();

	return 0;
}

int Apex::Audio::Lua_AudioComp_SetSound(lua_State* L)
{
	AudioComponent* comp = Comp::GetComponent<AudioComponent>(L, 1);

	const char* soundName = luaL_checkstring(L, 2);
	if (comp) comp->SetSound(soundName);

	return 0;
}

int Apex::Audio::Lua_AudioComp_SetVolume(lua_State* L)
{
	AudioComponent* comp = Comp::GetComponent<AudioComponent>(L, 1);

	float volume = (float)luaL_checknumber(L, 2);
	if (comp) comp->SetVolume(volume);

	return 0;
}

int Apex::Audio::Lua_AudioComp_SetChannel(lua_State* L)
{
	AudioComponent* comp = Comp::GetComponent<AudioComponent>(L, 1);

	int channel = (int)luaL_checkinteger(L, 2);
	if (comp) comp->SetChannel(channel);

	return 0;
}

int Apex::Audio::Lua_AudioComp_SetLoop(lua_State* L)
{
	AudioComponent* comp = Comp::GetComponent<AudioComponent>(L, 1);

	bool loop = lua_toboolean(L, 2);
	if (comp) comp->SetLoop(loop);

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
