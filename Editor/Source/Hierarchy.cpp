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

    if (m_gui.BeginGridContextMenu())
    {
        if (m_gui.MenuItem("Create Empty"))
        {
            Object* obj = m_scene.CreateObject();
            obj->SetName("Object");

            m_selected = obj;
        }

        m_gui.EndPopup();
    }

    for (auto& obj : m_scene.GetObjects())
    {
        SceneNode* node = obj->GetSceneNodeRaw();

        if (node && node->GetParent() == nullptr)
            DrawNode(node);
    }

    if (m_pendingParent)
    {
        Object* newObj = m_scene.CreateObject();
        newObj->SetName("Object");

        m_pendingParent->AddChild(newObj->GetSceneNode());

        m_selected = newObj;
        m_pendingParent = nullptr;
    }

    if (m_pendingDelete)
    {
        DestroyObject(m_pendingDelete);
        m_pendingDelete = nullptr;
    }

    if (m_gui.BeginDropTarget())
    {
        if (const void* data = m_gui.AcceptDragPayload("SCENE_NODE"))
        {
            SceneNode* dragged = *(SceneNode**)data;
            dragged->SetParent(nullptr);
        }

        m_gui.EndDropTarget();
    }

    m_gui.EndPanel();
}

void Hierarchy::DrawNode(SceneNode* node)
{
    Object* obj = node->GetObject();
    bool hasChildren = !node->GetChildren().empty();

    TreeNodeFlags flags = TreeNodeFlags::SpanAvailWidth;

    if (!hasChildren)
        flags = flags | TreeNodeFlags::Leaf;

    if (obj == m_selected)
        flags = flags | TreeNodeFlags::Selected;

    std::string label = obj->GetName() + "##" + std::to_string(obj->GetId());
    bool open = m_gui.BeginTreeNode(label.c_str(), flags);

    if (m_gui.IsItemClicked())
        m_selected = obj;

    if (m_gui.BeginContextMenu(std::to_string(obj->GetId()).c_str()))
    {
        if (m_gui.MenuItem("Create Child"))
        {
            m_pendingParent = node;
        }

        if (!m_gui.IsDragDropActive() && m_gui.MenuItem("Delete"))
        {
                m_pendingDelete = obj;
        }

        m_gui.EndPopup();
    }

    if (m_gui.BeginDragSource())
    {
        SceneNode* payload = node;
        m_gui.SetDragPayload("SCENE_NODE", &payload, sizeof(SceneNode*));
        m_gui.Text(obj->GetName());
        m_gui.EndDragSource();
    }

    if (m_gui.BeginDropTarget())
    {
        if (m_gui.IsDragDropActive() &&
            strcmp(m_gui.GetDragDropPayloadType(), "SCENE_NODE") == 0)
        {
            Vector2 size = m_gui.GetItemRectSize();
            Vector2 min = m_gui.GetItemRectMin();
            Vector2 max = m_gui.GetItemRectMax();

            float itemTop = min[1];
            float itemBottom = max[1];

            float mouseY = m_gui.GetMousePos()[1];
            float threshold = size[1] * 0.25f;

            if (mouseY < itemTop + threshold)
            {
                DrawInsertLine(itemTop);
            }
            else if (mouseY > itemBottom - threshold)
            {
                DrawInsertLine(itemBottom);
            }
        }

        if (const void* data = m_gui.AcceptDragPayload("SCENE_NODE"))
        {
            SceneNode* dragged = *(SceneNode**)data;

            Vector2 min = m_gui.GetItemRectMin();
            Vector2 max = m_gui.GetItemRectMax();

            float itemTop = min[1];
            float itemBottom = max[1];
            float height = itemBottom - itemTop;

            float mouseY = m_gui.GetMousePos()[1];
            float threshold = height * 0.25f;

            if (mouseY < itemTop + threshold)
                dragged->InsertBefore(node);
            else if (mouseY > itemBottom - threshold)
                dragged->InsertAfter(node);
            else
                dragged->SetParent(node);
        }

        m_gui.EndDropTarget();
    }

    if (open)
    {
        for (auto& child : node->GetChildren())
            DrawNode(child.get());

        m_gui.EndTreeNode();
    }
}

void Hierarchy::DrawInsertLine(float y)
{
    IDrawList* draw = m_gui.GetDrawList();

    Vector2 panelPos = m_gui.GetPanelPos();
    Vector2 panelSize = m_gui.GetPanelSize();

    float x1 = panelPos[0];
    float x2 = panelPos[0] + panelSize[0];

    draw->AddLine(
        { x1, y },
        { x2, y },
        PackColor(80, 160, 255),
        2.0f
    );
}

void Hierarchy::DestroyObject(Object* obj)
{
    SceneNode* node = obj->GetSceneNodeRaw();

    if (node->GetParent())
        node->GetParent()->DestroyChild(node);

    m_scene.DestroyObject(obj);

    if (m_selected == obj)
        m_selected = nullptr;
}