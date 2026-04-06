#ifndef INPUT_SYSTEM
#define INPUT_SYSTEM

#include <unordered_map>

#include "LibMath/Vector/Vector2.h"

namespace Apex::Input
{
	enum class Key
	{
		W, A, S, D, Q, E, P, F,
		Up, Down, Left, Right,
		Escape, Space,
		Unknown
	};

	enum class MouseButton
	{
		Left,
		Right,
		Middle
	};

	class InputSystem
	{
	public:
		InputSystem() = default;
		InputSystem(InputSystem const&) = default;
		InputSystem& operator=(InputSystem const&) = default;
		~InputSystem() = default;


		static InputSystem& Get();

		void SetKeyDown(Key key, bool down);
		bool IsKeyDown(Key key);
		void SetMouseButtonDown(MouseButton mouseButton, bool down);
		bool IsMouseButtonDown(MouseButton mouseButton);

		void SetMousePos(LibMath::Vector2 mousePos);
		LibMath::Vector2 GetMousePos() const;
		LibMath::Vector2 GetMouseDelta() const;

		void SetMouseScroll(float scroll);
		float GetMouseScroll(float scroll) const;

	private:
		std::unordered_map<Key, bool> m_key;
		std::unordered_map<MouseButton, bool> m_mouseButton;

		LibMath::Vector2 m_mousePos;
		LibMath::Vector2 m_mouseDelta;

		float m_mouseScroll = 0.f;
	};
}


#endif // !INPUT_SYSTEM

