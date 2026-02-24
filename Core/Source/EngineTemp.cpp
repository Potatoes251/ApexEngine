#include <LowRendererTemp.h>

#include <imgui.h>

#include <irrKlang.h>

#include <fbxsdk.h>

#include "PxPhysicsAPI.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

#include "LuaBehaviour.h"

#include <chrono>

int main()
{
	FbxManager* manager = FbxManager::Create();

	if (!manager)
	{
		std::cerr << "FBX SDK manager creation failed." << std::endl;
		return 1;
	}

	// Create IO settings (required by many FBX operations)
	FbxIOSettings* ioSettings = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ioSettings);

	std::cout << "FBX SDK is correctly included and linked." << std::endl;

	// Proper cleanup
	manager->Destroy();


	lua_State* luaState = luaL_newstate();

	if (!luaState)
	{
		std::cerr << "LuaJIT state creation failed." << std::endl;
		return 1;
	}

	luaL_openlibs(luaState);

	if (luaL_dofile(luaState, "../../Assets/Scripts/Script.lua") != LUA_OK)
	{
		std::cerr << "Lua error: "
			<< lua_tostring(luaState, -1)
			<< std::endl;
		return 1;
	}

	int ref = luaL_ref(luaState, LUA_REGISTRYINDEX);

	Apex::Scripting::LuaBehaviour luaBehaviour(luaState, ref);

	luaBehaviour.Start();

	auto lastTime = std::chrono::high_resolution_clock::now();

	while (true)
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed = currentTime - lastTime;

		float deltaTime = elapsed.count();
		lastTime = currentTime;

		luaBehaviour.Update(deltaTime);
	}

	lua_close(luaState);

	return 0;
}