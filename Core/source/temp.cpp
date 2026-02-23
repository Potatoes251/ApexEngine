#include <imgui.h>
#include <irrKlang.h>
#include <fbxsdk.h>
#include "PxPhysicsAPI.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

#include <lua.hpp>

#include <temp.h>

int main()
{
	IMGUI_CHECKVERSION();
	ImGuiContext* context = ImGui::CreateContext();

	if (!context)
	{
		printf("ImGui context creation failed.\n");
		return 1;
	}

	printf("ImGui is correctly included and linked.\n");

	ImGui::DestroyContext(context);


	irrklang::ISoundEngine* soundEngine = irrklang::createIrrKlangDevice();

	if (!soundEngine)
	{
		std::cerr << "IrrKlang engine creation failed." << std::endl;
		return 1;
	}

	std::cout << "IrrKlang is correctly included and linked." << std::endl;

	soundEngine->drop(); // Properly release the engine

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

	using namespace physx;

	PxDefaultAllocator		allocator;
	PxDefaultErrorCallback	errorCallback;

	PxFoundation* foundation = PxCreateFoundation(
		PX_PHYSICS_VERSION,
		allocator,
		errorCallback
	);

	if (!foundation)
	{
		std::cerr << "PhysX foundation creation failed." << std::endl;
		return 1;
	}

	PxTolerancesScale scale;

	PxPhysics* physics = PxCreatePhysics(
		PX_PHYSICS_VERSION,
		*foundation,
		scale,
		true,
		nullptr
	);

	if (!physics)
	{
		std::cerr << "PhysX physics creation failed." << std::endl;
		foundation->release();
		return 1;
	}

	std::cout << "PhysX is correctly included and linked." << std::endl;

	// Proper cleanup
	physics->release();
	foundation->release();


	lua_State* luaState = luaL_newstate();

	if (!luaState)
	{
		std::cerr << "LuaJIT state creation failed." << std::endl;
		return 1;
	}

	luaL_openlibs(luaState);

	// Run a small Lua script
	const char* luaScript = "return 2 + 3";

	if (luaL_dostring(luaState, luaScript) != LUA_OK)
	{
		std::cerr << "LuaJIT script execution failed: "
			<< lua_tostring(luaState, -1) << std::endl;
		lua_close(luaState);
		return 1;
	}

	// Get result from stack
	if (!lua_isnumber(luaState, -1))
	{
		std::cerr << "LuaJIT did not return a number." << std::endl;
		lua_close(luaState);
		return 1;
	}

	double result = lua_tonumber(luaState, -1);

	std::cout << "LuaJIT executed script successfully. Result: "
		<< result << std::endl;

	lua_close(luaState);

	std::cout << "LuaJIT is correctly included and linked." << std::endl;

	return 0;
}