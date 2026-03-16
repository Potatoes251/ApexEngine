#ifndef HERARCHY
#define HERARCHY

#include "UI.h"
#include "Window.h"

#include "Scene.h"
#include "Object.h"
#include "SceneGraph.h"

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
			Apex::Rendering::Scene& scene);

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

		void DestroyObject(Object* obj);

	private:
		Apex::UserInterface::IGUI& m_gui;
		Apex::Rendering::Scene& m_scene;

		Apex::Data::Object* m_selected = nullptr;
		Apex::SceneGraph::SceneNode* m_pendingParent = nullptr;
		Apex::Data::Object* m_pendingDelete = nullptr;

		Apex::SceneGraph::SceneNode* m_pendingReparent = nullptr;
		Apex::SceneGraph::SceneNode* m_pendingTarget = nullptr;

		PendingOp m_pendingOp = PendingOp::None;
	};
}

#endif // !HERARCHY
