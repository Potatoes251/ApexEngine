#ifndef SCRIPT_PICKER
#define SCRIPT_PICKER

#include "UI.h"
#include "ScriptComponent.h"


namespace Apex::Rendering { class Scene; }

namespace Apex::Resources
{
    class ScriptPicker
    {
    public:
        ScriptPicker(UserInterface::IGUI* gui) : m_gui(gui) {}

        void Open(Rendering::Scene* scene, Scripting::ScriptComponent* target);

        void Draw();

        bool HasNewScript() const { return m_newScript; }
        void ResetNewScript() { m_newScript = false; }

    private:
        bool m_isOpen = false;
        bool m_newScript = false;
        Apex::Scripting::ScriptComponent* m_target = nullptr;
        Apex::UserInterface::IGUI* m_gui;
        Rendering::Scene* m_scene = nullptr;
    };
}

#endif // !SCRIPT_PICKER