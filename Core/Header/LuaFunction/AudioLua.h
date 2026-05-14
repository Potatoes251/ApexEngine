#ifndef AUDIO_LUA
#define AUDIO_LUA

#include <lua.hpp>

#include "LuaManager.h"

namespace Apex::Audio
{
	class IAudioEngine;
	class AudioComponent;

	void RegisterLua(Scripting::LuaManager& lua);

    AudioComponent* GetAudioComponent(lua_State* L, int index);

    int Lua_AudioComp_Play(lua_State* L);
    int Lua_AudioComp_Play3D(lua_State* L);
    int Lua_AudioComp_PlayAt(lua_State* L);

    int Lua_GetAudioComponent(lua_State* L);
}

#endif // !AUDIO_LUA