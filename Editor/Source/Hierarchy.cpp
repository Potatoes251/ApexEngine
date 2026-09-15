#include "Hierarchy.h"

#include "Viewport.h"
#include "Scene.h"
#include "Object.h"

#include "Log.h"

#include "Command/RemoveObjectCommand.h"
#include "Command/AddObjectCommand.h"
#include "CommandManager.h"

#include "Application.h"

using namespace Apex::Editor;
using namespace Apex::SceneGraph;
using namespace Apex::UserInterface;
using namespace Apex::Rendering;
using namespace Apex::Data;
using namespace Apex::Windowing;
using namespace Apex::CtrlZ;
using namespace Apex::Resources;

Hierarchy::Hierarchy(IGUI& gui,
    Scene* scene, ComponentsViewer* compViewer, IWindow& window, ResourceManager* resourceManager)
    : m_gui(gui), m_scene(scene), m_componentsViewer(compViewer), m_window(window), m_resourceManager(resourceManager)
{
}

void Hierarchy::Draw()
{
    m_scene = Application::Get()->GetScene();
    m_gui.PushFont(FontID::Default);

    m_gui.BeginPanel("Hierarchy");

    m_isFocused = m_gui.IsPanelFocused();

    if (m_gui.Button(" + Add "))
        m_gui.OpenPopup("AddObject");

    if (m_gui.BeginPopup("AddObject"))
    {
        if (m_gui.MenuItem("Empty"))
        {
            CommandManager::Get().Execute(std::make_unique<AddObjectCommand>(
                [this]()
                {
                    Object* obj = m_scene->CreateObject();
                    SelectObject(obj);
                    return obj;
                },
                [this](size_t objId)
                {
                    DestroyObject(objId);
                }));
        }

        if (m_gui.MenuItem("Cube"))
        {
            Object* obj = m_scene->CreateObject();
            auto& comp = obj->AddComponent<MeshRenderer>();

            auto mesh = m_resourceManager->CreateAsync<Model>("ApexAssets/Meshes/Cube/Cube.mesh");
            comp.SetModel(mesh);
            auto mat = m_resourceManager->CreateAsync<Material>("Assets/Material/Default.mat");
            comp.SetMaterials({ mat });
            obj->SetName("Cube");

            SelectObject(obj);
        }

        if (m_gui.MenuItem("Sphere"))
        {
            Object* obj = m_scene->CreateObject();
            auto& comp = obj->AddComponent<MeshRenderer>();

            auto mesh = m_resourceManager->CreateAsync<Model>("ApexAssets/Meshes/Sphere/Sphere.mesh");
            comp.SetModel(mesh);
            auto mat = m_resourceManager->CreateAsync<Material>("Assets/Material/Default.mat");
            comp.SetMaterials({ mat });
            obj->SetName("Sphere");

            SelectObject(obj);
        }
        m_gui.EndPopup();
    }

    for (auto& obj : m_scene->GetObjects())
    {
        SceneNode* node = obj->GetSceneNodeRaw();
        if (node && node->GetParent() == nullptr)
            DrawNode(node);
    }

    HandlePending();

    if (m_selected && m_gui.IsPanelFocused() && (m_gui.IsKeyPressed(UIKey::F2) || m_gui.IsItemDoubleClicked()))
    {
        Rename();
    }

    Vector2 size = m_gui.GetAvailableSize();
    m_gui.InvisibleButton("HierarchyDropArea", size);

    if (m_gui.IsItemClicked())
    {
        SelectObject(nullptr);
    }

    if (m_gui.BeginDropTarget())
    {
        if (const void* data = m_gui.AcceptDragPayload("SCENE_NODE"))
        {
            SceneNode* dragged = *(SceneNode**)data;

            if (m_pendingOp == PendingOp::None)
            {
                m_pendingReparent = dragged;
                m_pendingTarget = nullptr;
                m_pendingOp = PendingOp::Parent;
            }
        }

        m_gui.EndDropTarget();
    }

    m_gui.EndPanel();
    m_gui.PopFont();

    ApplyPendingOp();
}

void Apex::Editor::Hierarchy::Rename()
{
    m_renaming = m_selected;
    m_renameFocusPending = true;
    strncpy(m_renameBuffer, m_selected->GetName().c_str(), sizeof(m_renameBuffer));
    m_renameBuffer[sizeof(m_renameBuffer) - 1] = '\0';
}

void Hierarchy::SelectObject(Apex::Data::Object* selected, bool send)
{
    m_selected = selected;
    if (send && m_viewport)
        m_viewport->SelectObject(selected, false);

    if (m_componentsViewer)
        m_componentsViewer->SetSelectedObject(selected);
}

void Apex::Editor::Hierarchy::ClearSelection()
{
    SelectObject(nullptr);
    m_pendingClone = nullptr;
    m_pendingDelete = nullptr;
    m_pendingParent = nullptr;
    m_pendingReparent = nullptr;
    m_pendingTarget = nullptr;
    m_renaming = nullptr;
}

void Hierarchy::DrawNode(SceneNode* node)
{
    Object* obj = node->GetObject();
    bool open = DrawTreeNode(obj, node->GetChildren().empty());

    HandleSelection(obj);
    HandleContextMenu(node);
    HandleDragSource(node);
    HandleDropTarget(node);

    if (open)
    {
        for (auto& child : node->GetChildren())
            DrawNode(child.get());

        m_gui.EndTreeNode();
    }
}

void Apex::Editor::Hierarchy::HandlePending()
{
    if (m_pendingParent)
    {
        Object* newObj = m_scene->CreateObject();
        m_pendingParent->AddChild(newObj->GetSceneNode());
        SelectObject(newObj);
        m_pendingParent = nullptr;
    }

    if (m_pendingDelete)
    {
        CommandManager::Get().Execute(std::make_unique<RemoveObjectCommand>(m_pendingDelete,
            [this](std::unique_ptr<Object> obj)
            {
                m_scene->AddObject(std::move(obj));
            },
            [this](size_t objId)
            {
                DestroyObject(objId);
            }));
        m_pendingDelete = nullptr;
    }

    if (m_pendingClone)
    {
        Object* clone = DuplicateObject(m_pendingClone);
        SelectObject(clone);
        m_pendingClone = nullptr;
    }
}

bool Hierarchy::DrawTreeNode(Object* obj, bool isLeaf)
{
    TreeNodeFlags flags = TreeNodeFlags::SpanAvailWidth;
    if (isLeaf) flags = flags | TreeNodeFlags::Leaf;
    if (obj == m_selected) flags = flags | TreeNodeFlags::Selected;
    std::string id = "##" + std::to_string(obj->GetId());

    if (obj != m_renaming)
    {
        std::string label = obj->GetName() + "##" + std::to_string(obj->GetId());
        return m_gui.BeginTreeNode(label.c_str(), flags);
    }

    if (m_renameFocusPending)
    {
        m_gui.SetKeyboardFocusHere();
        m_renameFocusPending = false;
    }

    m_gui.InputText(id, m_renameBuffer, sizeof(m_renameBuffer));

    if (m_gui.IsKeyPressed(UIKey::Enter))
    {
        bool voidName = true;

        for (size_t i = 0; m_renameBuffer[i] != '\0'; i++)
        {
            if (m_renameBuffer[i] != ' ' &&
                m_renameBuffer[i] != '\t' &&
                m_renameBuffer[i] != '\n' &&
                m_renameBuffer[i] != '\r' &&
                m_renameBuffer[i] != '\v' &&
                m_renameBuffer[i] != '\f')
            {
                voidName = false;
                break;
            }
        }

        size_t start = 0;

        while (m_renameBuffer[start] != '\0' &&
            (m_renameBuffer[start] == ' ' ||
                m_renameBuffer[start] == '\t' ||
                m_renameBuffer[start] == '\n' ||
                m_renameBuffer[start] == '\r' ||
                m_renameBuffer[start] == '\v' ||
                m_renameBuffer[start] == '\f'))
        {
            start++;
        }

        if (start > 0)
        {
            size_t i = 0;
            while (m_renameBuffer[start + i] != '\0')
            {
                m_renameBuffer[i] = m_renameBuffer[start + i];
                i++;
            }
            m_renameBuffer[i] = '\0';
        }

        if (!voidName) obj->SetName(m_renameBuffer);
        m_renaming = nullptr;
    }
    if (m_gui.IsKeyPressed(UIKey::Escape))
        m_renaming = nullptr;

    return false;
}

void Hierarchy::HandleSelection(Object* obj)
{
    if (m_gui.IsItemClicked())
    {
        SelectObject(obj);
    }
}

void Hierarchy::HandleContextMenu(SceneNode* node)
{
    Object* obj = node->GetObject();
    if (!m_gui.BeginContextMenu(std::to_string(obj->GetId()).c_str()))
        return;

    if (m_gui.MenuItem("Create Child"))
        m_pendingParent = node;

    if (m_gui.MenuItem("Rename"))
        Rename();

    if (m_gui.MenuItem("Duplicate"))
        m_pendingClone = obj;

    if (!m_gui.IsDragDropActive() && m_gui.MenuItem("Delete"))
        m_pendingDelete = obj;

    m_gui.EndPopup();
}

void Hierarchy::HandleDragSource(SceneNode* node)
{
    if (!m_gui.BeginDragSource()) return;

    SceneNode* payload = node;
    m_gui.SetDragPayload("SCENE_NODE", &payload, sizeof(SceneNode*));
    m_gui.Text(node->GetObject()->GetName());
    m_gui.EndDragSource();
}

void Hierarchy::HandleDropTarget(SceneNode* node)
{
    if (!m_gui.BeginDropTarget()) return;

    HandleInsertLine(node);
    HandleAcceptDrop(node);

    m_gui.EndDropTarget();
}

void Hierarchy::HandleInsertLine(SceneNode* node)
{
    if (!m_gui.IsDragDropActive() ||
        strcmp(m_gui.GetDragDropPayloadType(), "SCENE_NODE") != 0)
        return;

    Vector2 size = m_gui.GetItemRectSize();
    Vector2 min = m_gui.GetItemRectMin();
    Vector2 max = m_gui.GetItemRectMax();

    float itemTop = min[1];
    float itemBottom = max[1];
    float mouseY = m_gui.GetMousePos()[1];
    float threshold = size[1] * 0.25f;

    if (mouseY < itemTop + threshold)
        DrawInsertLine(itemTop);
    else if (mouseY > itemBottom - threshold)
        DrawInsertLine(itemBottom);
}

void Hierarchy::HandleAcceptDrop(SceneNode* node)
{
    if (const void* data = m_gui.AcceptDragPayload("SCENE_NODE"))
    {
        SceneNode* dragged = *(SceneNode**)data;
        if (dragged == node) return;

        Vector2 min = m_gui.GetItemRectMin();
        Vector2 max = m_gui.GetItemRectMax();
        float itemTop = min[1], itemBottom = max[1], height = itemBottom - itemTop;
        float mouseY = m_gui.GetMousePos()[1];
        float threshold = height * 0.25f;

        if (mouseY < itemTop + threshold)
        {
            SetPending(dragged, node, PendingOp::Before);
        }
        else if (mouseY > itemBottom - threshold)
        {
            SetPending(dragged, node, PendingOp::After);
        }
        else if (!node->IsDescendantOf(dragged))
        {
            SetPending(dragged, node, PendingOp::Parent);
        }
    }
}

void Hierarchy::SetPending(SceneNode* dragged, SceneNode* target, PendingOp op)
{
    m_pendingReparent = dragged;
    m_pendingTarget = target;
    m_pendingOp = op;
}

void Hierarchy::ApplyPendingOp()
{
    if (m_pendingOp == PendingOp::None)
        return;

    if (m_pendingOp == PendingOp::Before)
    {
        if (m_pendingReparent->GetParent())
            m_pendingReparent->SetParent(nullptr);

        m_scene->InsertBefore(
            m_pendingReparent->GetObject(),
            m_pendingTarget->GetObject());
    }
    else if (m_pendingOp == PendingOp::After)
    {
        if (m_pendingReparent->GetParent())
            m_pendingReparent->SetParent(nullptr);

        m_scene->InsertAfter(
            m_pendingReparent->GetObject(),
            m_pendingTarget->GetObject());
    }
    else if (m_pendingOp == PendingOp::Parent)
    {
        if (m_pendingTarget)
            m_pendingReparent->SetParent(m_pendingTarget);
        else if (m_pendingReparent && m_pendingReparent->GetParent())
            m_pendingReparent->SetParent(nullptr);
    }

    m_pendingOp = PendingOp::None;
    m_pendingReparent = nullptr;
    m_pendingTarget = nullptr;
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

Object* Hierarchy::DuplicateObject(Object* original)
{
    std::unique_ptr<Object> clone = original->Clone();
    size_t id = clone->GetId();
    m_scene->AddObject(std::move(clone));

    return m_scene->GetObjectWithId(id);
}

void Hierarchy::DestroyObject(size_t objId)
{
    SelectObject(nullptr);
    Data::Object* obj = m_scene->GetObjectWithId(objId);

    if (m_selected == obj)
        SelectObject(nullptr);

    m_scene->DestroyObject(objId);
}

// Ctrl+D — duplicate the currently selected object (same as context-menu Duplicate).
void Hierarchy::DuplicateSelected()
{
    if (!m_selected) return;
    m_pendingClone = m_selected;
}

// Ctrl+V — paste a previously copied object into the scene as a new duplicate.
// Source is the object that was selected at Ctrl+C time.
void Hierarchy::PasteObject(Data::Object* source)
{
    if (!source) return;
    Object* clone = DuplicateObject(source);
    SelectObject(clone);
}