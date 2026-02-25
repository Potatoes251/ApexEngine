#include <LowRendererTemp.h>

#include <imgui.h>

#include <irrKlang.h>

#include <fbxsdk.h>

#include "PxPhysicsAPI.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

#include "LuaManager.h"
#include "LuaBehaviour.h"

#include <chrono>

int main()
{
	Apex::Scripting::LuaManager luaManager;

	luaManager.LoadScript("../../Assets/Scripts/Script.lua");

	auto lastTime = std::chrono::high_resolution_clock::now();

	while (true)
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed = currentTime - lastTime;

		float deltaTime = elapsed.count();
		lastTime = currentTime;

		luaManager.CallUpdateAll(deltaTime);
	}

	return 0;
}