#ifndef GAME
#define GAME

#include "Application.h"
#include "Window.h"

namespace Apex::Game
{
	class Game
	{
    public:
        Game() = default;
        ~Game() = default;

        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;

        bool Init();

        bool InitWindow();

        void Update();

        void Render();

        void SaveGame();
        void SaveCurrentGame();
        void LoadSavedGame();

        void SetCurrentSaveSlot(int slot) { m_currentSaveSlot = slot; }
        int GetCurrentSaveSlot() const { return m_currentSaveSlot; }
        void AddSaveSlot() { std::ofstream file(m_app->GetSaveManager()->BuildPath(m_saveSlotCount++)); }
        void DeleteSaveSlot(int slot);

        Apex::Windowing::IWindow* GetWindow() { return m_window.get(); }

	private:
        std::unique_ptr<Apex::Application>          m_app;
        std::unique_ptr<Apex::Windowing::IWindow>	m_window;
        std::unique_ptr<Apex::UserInterface::IGUI>	m_gui;

        int m_currentSaveSlot = -1;
        int m_saveSlotCount = 0;
	};
}

#endif // !GAME
