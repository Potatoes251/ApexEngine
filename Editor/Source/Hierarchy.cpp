#include "Hierarchy.h"

using namespace Apex::Editor;
using namespace Apex::SceneGraph;
using namespace Apex::UserInterface;
using namespace Apex::Rendering;
using namespace Apex::Data;

Hierarchy::Hierarchy(IGUI& gui,
    Scene& scene)
    : m_gui(gui), m_scene(scene)
{
}

void Hierarchy::Draw()
{
    m_gui.BeginPanel("Hierarchy");

    for (auto& obj : m_scene.GetObjects())
    {
        SceneNode* node = obj->GetSceneNodeRaw();

        if (node && node->GetParent() == nullptr)
            DrawNode(node);
    }

    m_gui.EndPanel();
}

void Hierarchy::DrawNode(SceneNode* node)
{
    Object* obj = node->GetObject();

    bool hasChildren = !node->GetChildren().empty();

    TreeNodeFlags flags =
        TreeNodeFlags::SpanAvailWidth;

    if (!hasChildren)
        flags = flags | TreeNodeFlags::Leaf;

    if (obj == m_selected)
        flags = flags | TreeNodeFlags::Selected;

    bool open = m_gui.BeginTreeNode(obj->GetName(), flags);

    if (m_gui.IsItemClicked())
        m_selected = obj;

    if (open)
    {
        for (auto& child : node->GetChildren())
            DrawNode(child.get());

        m_gui.EndTreeNode();
    }
}