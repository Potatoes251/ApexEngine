#include "LuaFunction/ShowMouseLua.h"

#include "Application.h"
#include "Window.h"

namespace Apex::Mouse
{
	void Apex::Mouse::RegisterLua(Scripting::LuaManager& lua)
	{
		lua.RegisterFunction("ShowMouse", Apex::Mouse::Lua_ShowMouse);
		lua.RegisterFunction("HideMouse", Apex::Mouse::Lua_HideMouse);
	}

	int Apex::Mouse::Lua_ShowMouse(lua_State* L)
	{
		Application::Get()->GetWindow()->SetCursorCaptured(false);

		return 0;
	}

	int Apex::Mouse::Lua_HideMouse(lua_State* L)
	{
		Application::Get()->GetWindow()->SetCursorCaptured(true);

		return 0;
	}
}