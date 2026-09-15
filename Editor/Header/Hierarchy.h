#ifndef HERARCHY
#define HERARCHY

#include "UI.h"

namespace Apex::Rendering
{
	class Scene;
}
namespace Apex::SceneGraph
{
	class SceneNode;
}
namespace Apex::Data
{
	class Object;
}

#include "ComponentsViewer.h"

#include "Window.h"

#include "ResourceManager.h"

namespace Apex::Editor
{
	class Viewport;

	enum class PendingOp
	{
		None,
		Before,
		After,
		Parent
	};

	class Hierarchy
	{
	public:
		Hierarchy(Apex::UserInterface::IGUI& gui, Apex::Rendering::Scene* scene,
			ComponentsViewer* compViewer, Apex::Windowing::IWindow& window,
			Apex::Resources::ResourceManager* resourceManager);

		void Draw();
		void Rename();
		void SetScene(Rendering::Scene* newScene) { m_scene = newScene; }
		void SetViewport(Viewport* viewport) { m_viewport = viewport; }
		void SelectObject(Apex::Data::Object* selected, bool send = true);
		Apex::Data::Object* GetSelected() const { return m_selected; }
		void ClearSelection();

		// Clipboard / shortcut helpers called by Editor.
		bool IsFocused()    const { return m_isFocused; }
		void DuplicateSelected();
		void PasteObject(Apex::Data::Object* source);

	private:
		void DrawNode(Apex::SceneGraph::SceneNode* node);

		void HandlePending();

		bool DrawTreeNode(Apex::Data::Object* obj, bool isLeaf);

		void HandleSelection(Apex::Data::Object* obj);
		void HandleContextMenu(Apex::SceneGraph::SceneNode* node);
		void HandleDragSource(Apex::SceneGraph::SceneNode* node);
		void HandleDropTarget(Apex::SceneGraph::SceneNode* node);
		void HandleInsertLine(Apex::SceneGraph::SceneNode* node);
		void HandleAcceptDrop(Apex::SceneGraph::SceneNode* node);

		void SetPending(Apex::SceneGraph::SceneNode* dragged, Apex::SceneGraph::SceneNode* target, PendingOp op);

		void DrawInsertLine(float y);

		void ApplyPendingOp();

		Apex::Data::Object* DuplicateObject(Apex::Data::Object* original);

		void DestroyObject(size_t objId);

		Apex::UserInterface::IGUI& m_gui;
		Apex::Rendering::Scene* m_scene;
		ComponentsViewer* m_componentsViewer;
		Apex::Resources::ResourceManager* m_resourceManager;

		Apex::Windowing::IWindow& m_window;

		Apex::Data::Object* m_selected = nullptr;
		Apex::SceneGraph::SceneNode* m_pendingParent = nullptr;
		Apex::Data::Object* m_pendingDelete = nullptr;

		Apex::SceneGraph::SceneNode* m_pendingReparent = nullptr;
		Apex::SceneGraph::SceneNode* m_pendingTarget = nullptr;

		Apex::Data::Object* m_pendingClone = nullptr;
		Viewport* m_viewport = nullptr;

		PendingOp m_pendingOp = PendingOp::None;

		bool m_isFocused = false;

		Apex::Data::Object* m_renaming = nullptr;
		char m_renameBuffer[256] = {};
		bool m_renameFocusPending = false;
	};
}

#endif // !HERARCHY