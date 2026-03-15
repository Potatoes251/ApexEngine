#ifndef HERARCHY
#define HERARCHY

#include "UI.h"
#include "Window.h"

#include "Scene.h"
#include "Object.h"
#include "SceneGraph.h"

namespace Apex::Editor
{
	class Hierarchy
	{
	public:
		Hierarchy(Apex::UserInterface::IGUI& gui,
			Apex::Rendering::Scene& scene);

		void Draw();

	private:
		void DrawNode(Apex::SceneGraph::SceneNode* node);

		void DrawInsertLine(float y);

	private:
		Apex::UserInterface::IGUI& m_gui;
		Apex::Rendering::Scene& m_scene;

		Apex::Data::Object* m_selected = nullptr;
	};
}

#endif // !HERARCHY
