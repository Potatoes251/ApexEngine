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

        void Open(Apex::Scripting::ScriptComponent* target)
        {
            m_target = target;
            m_isOpen = true;
        }

        void Draw()
        {
            if (!m_isOpen)
                return;

            m_gui.BeginPanel("Select Script", &m_isOpen);

            m_gui.Text("Working dir: " + std::filesystem::current_path().string());

            if (!std::filesystem::exists("Assets/Scripts/"))
            {
                m_gui.Text("Path not found!");
                m_gui.EndPanel();
                return;
            }

            try
            {
                for (auto& entry : std::filesystem::recursive_directory_iterator("Assets/Scripts/"))
                {
                    if (!entry.is_regular_file())
                        continue;

                    m_gui.Text(entry.path().string());

                    if (entry.path().extension() != ".lua")
                        continue;

                    std::string path = entry.path().generic_string();

                    if (m_gui.Button(path.c_str()))
                    {
                        if (m_target)
                        {
                            m_target->SetScript(path);
                        }

                        m_isOpen = false;
                    }
                }
            }
            catch (std::exception& e)
            {
                m_gui.Text(std::string("Error: ") + e.what());
            }

            m_gui.EndPanel();
        }

    private:
        bool m_isOpen = false;
        Apex::Scripting::ScriptComponent* m_target = nullptr;
        Apex::UserInterface::IGUI& m_gui;
    };
}

#endif // !SCRIPT_PICKER