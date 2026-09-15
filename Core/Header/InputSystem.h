#ifndef INPUT_SYSTEM
#define INPUT_SYSTEM

#include <unordered_map>

#include "LibMath/Vector/Vector2.h"

namespace Apex::Input
{
	enum class Key
	{
		A, B, C, D, E, F, G, H, I,
		J, K, L, M, N, O, P, Q, R,
		S, T, U, V, W, X, Y, Z,

		N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,

		Up, Down, Left, Right,
		Escape, Space, Tab, Enter, BackSpace,
		Shift, Ctrl,
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

