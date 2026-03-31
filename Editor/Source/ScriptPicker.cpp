#include "ScriptPicker.h"

using namespace Apex::Resources;

void ScriptPicker::Open(Apex::Scripting::ScriptComponent* target)
{
    m_target = target;
    m_isOpen = true;
}

void ScriptPicker::Draw()
{
    if (!m_isOpen)
        return;

    m_gui.BeginModalBlock("Select Script");

    m_gui.BeginPanel("Select Script", &m_isOpen);

    m_gui.Text("Working dir: " + std::filesystem::current_path().string());

    if (!std::filesystem::exists("Assets/Scripts/"))
    {
        m_gui.Text("Path not found!");
        m_gui.EndPanel();
        m_gui.EndModalBlock();
        m_isOpen = false;
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
                    if (m_target->GetScriptPath() != path)
                    {
                        m_target->SetScript(path);
                        m_newScript = true;
                    }
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

    if (!m_isOpen)
        m_gui.EndModalBlock();
}