#ifndef SCRIPT_PICKER
#define SCRIPT_PICKER

#include "UI.h"
#include "ScriptComponent.h"

#include <filesystem>

namespace Apex::Resources
{
    class ScriptPicker
    {
    public:
        ScriptPicker(Apex::UserInterface::IGUI& gui) : m_gui(gui) {}

        void Open(Apex::Scripting::ScriptComponent* target);

        void Draw();

        bool HasNewScript() { return m_newScript; }
        void ResetNewScript() { m_newScript = false; }

    private:
        bool m_isOpen = false;
        bool m_newScript = false;
        Apex::Scripting::ScriptComponent* m_target = nullptr;
        Apex::UserInterface::IGUI& m_gui;
    };
}

#endif // !SCRIPT_PICKER