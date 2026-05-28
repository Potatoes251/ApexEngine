#include "ScriptPicker.h"
#include "AssetPicker.h"
#include "EditorIcon.h"

#include "Command/SetScriptPathCommand.h"
#include "CommandManager.h"

#include <filesystem>

using namespace Apex::Resources;
using namespace Apex::Rendering;
using namespace Apex::UserInterface;

void ScriptPicker::Open(Rendering::Scene* scene, Apex::Scripting::ScriptComponent* target)
{
    m_scene = scene;
    m_target = target;
    m_isOpen = true;
}

static std::string DrawScriptGrid(IGUI* gui, int cols)
{
    uint32_t scriptIcon = Apex::Editor::EditorIcon::Get("ApexAssets/Icons/script.png");

    gui->BeginChildPanel("##sp_grid", 0.f, -30.f, false);

    int col = 0;
    try
    {
        for (auto& entry : std::filesystem::recursive_directory_iterator("Assets/"))
        {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".lua") continue;

            std::string path = entry.path().generic_string();

            if (col > 0 && col % cols != 0)
                gui->SameLine(0.f, Apex::Resources::PICKER_SPACING);

            gui->PushID(path.c_str());
            bool picked = DrawPickerCell(gui, path, scriptIcon);
            gui->PopID();

            if (picked)
            {
                gui->EndChildPanel();
                return path;
            }
            col++;
        }
    }
    catch (std::exception& e)
    {
        gui->Text(std::string("Error: ") + e.what());
    }

    gui->EndChildPanel();
    return {};
}

static void CommitScriptSelection(
    Apex::Scripting::ScriptComponent* target,
    Apex::Rendering::Scene* scene,
    const std::string& newPath)
{
    if (target->GetScriptPath() == newPath) return;

    if (scene)
    {
        Apex::CtrlZ::CommandManager::Get().Execute(
            std::make_unique<Apex::CtrlZ::SetScriptPathCommand>(target, target->GetScriptPath(), newPath));
    }
    else
    {
        target->SetScript(newPath);
    }
}

void ScriptPicker::Draw()
{
    if (!m_isOpen) return;

    if (!std::filesystem::exists("Assets/"))
    {
        m_isOpen = false;
        return;
    }

    m_gui->BeginModalBlock("Select Script");
    m_gui->BeginPanel("Select Script", &m_isOpen, false, true);

    int cols = PickerColumnCount(m_gui->GetAvailableSize()[0]);

    std::string picked = DrawScriptGrid(m_gui, cols);
    if (!picked.empty() && m_target)
    {
        CommitScriptSelection(m_target, m_scene, picked);
        m_newScript = true;
        m_isOpen = false;
        m_gui->EndPanel();
        m_gui->EndModalBlock();
        return;
    }

    m_gui->EndPanel();
    m_gui->EndModalBlock();
}