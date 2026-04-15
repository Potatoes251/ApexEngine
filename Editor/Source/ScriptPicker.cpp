#include "ScriptPicker.h"

#include "Command/SetScriptPathCommand.h"
#include "CommandManager.h"

#include <filesystem>

using namespace Apex::Resources;
using namespace Apex::Rendering;

void ScriptPicker::Open(Rendering::Scene* scene, Apex::Scripting::ScriptComponent* target)
{
    m_scene = scene;
    m_target = target;
    m_isOpen = true;
}

void ScriptPicker::Draw()
{
    if (!m_isOpen)
        return;

    m_gui.BeginModalBlock("Select Script");

    m_gui.BeginPanel("Select Script", &m_isOpen, false, true);

    if (!std::filesystem::exists("Assets/"))
    {
        m_gui.Text("Path not found!");
        m_gui.EndPanel();
        m_gui.EndModalBlock();
        m_isOpen = false;
        return;
    }

    try
    {
        for (auto& entry : std::filesystem::recursive_directory_iterator("Assets/"))
        {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().extension() != ".lua")
                continue;

            std::string path = entry.path().generic_string();

            if (m_gui.Button(path.c_str()))
            {
                if (m_target)
                {
                    if (m_target->GetScriptPath() != path)
                    {
                        CtrlZ::CommandManager::Get().Execute(std::make_unique<CtrlZ::SetScriptPathCommand>(m_scene, m_target, m_target->GetScriptPath(), path));
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