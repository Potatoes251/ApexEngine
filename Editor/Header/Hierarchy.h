#ifndef HERARCHY
#define HERARCHY

#include "UI.h"
#include "Window.h"

#include "Scene.h"
#include "Object.h"
#include "SceneGraph.h"

#include "ComponentsViewer.h"

namespace Apex::Editor
{
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
		Hierarchy(Apex::UserInterface::IGUI& gui,
			Apex::Rendering::Scene& scene, ComponentsViewer* compViewer);

		void Draw();

	private:
		void DrawNode(Apex::SceneGraph::SceneNode* node);

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

		void DestroyObject(Object* obj);

	private:
		Apex::UserInterface::IGUI& m_gui;
		Apex::Rendering::Scene& m_scene;
		ComponentsViewer* m_componentsViewer;

		Apex::Data::Object* m_selected = nullptr;
		Apex::SceneGraph::SceneNode* m_pendingParent = nullptr;
		Apex::Data::Object* m_pendingDelete = nullptr;

		Apex::SceneGraph::SceneNode* m_pendingReparent = nullptr;
		Apex::SceneGraph::SceneNode* m_pendingTarget = nullptr;

		Apex::Data::Object* m_pendingClone = nullptr;

		PendingOp m_pendingOp = PendingOp::None;
	};
}

#endif // !HERARCHY
