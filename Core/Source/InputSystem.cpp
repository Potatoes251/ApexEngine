#include "InputSystem.h"

using namespace Apex::Input;


InputSystem& InputSystem::Get()
{
	static InputSystem inputSystem;

	return inputSystem;
}

void InputSystem::SetKeyDown(Key key, bool down)
{
	m_key[key] = down;
}

bool InputSystem::IsKeyDown(Key key)
{
	return m_key[key];
}

void InputSystem::SetMouseButtonDown(MouseButton mouseButton, bool down)
{
	m_mouseButton[mouseButton] = down;
}

bool InputSystem::IsMouseButtonDown(MouseButton mouseButton)
{
	return m_mouseButton[mouseButton];
}

void InputSystem::SetMousePos(LibMath::Vector2 mousePos)
{
	m_mouseDelta = mousePos - m_mousePos;
	m_mousePos = mousePos;
}

LibMath::Vector2 InputSystem::GetMousePos() const
{
	return m_mousePos;
}

LibMath::Vector2 InputSystem::GetMouseDelta() const
{
	return m_mouseDelta;
}

void InputSystem::SetMouseScroll(float scroll)
{
	m_mouseScroll = scroll;
}

float InputSystem::GetMouseScroll(float scroll) const
{
	return m_mouseScroll;
}
