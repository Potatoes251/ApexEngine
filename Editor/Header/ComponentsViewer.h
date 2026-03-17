#ifndef COMPONENTS_VIEWER
#define COMPONENTS_VIEWER

#include "UI.h"

#include "Scene.h"
#include "Object.h"
#include "Component.h"

namespace Apex::Editor
{
	class ComponentsViewer
	{
	public:
		ComponentsViewer(Apex::UserInterface::IGUI& gui,
			Apex::Rendering::Scene& scene);
		
		void SetSelectedObject(Apex::Data::Object* obj) { m_selectedObj = obj; }

		void Draw();

		void DrawComponent(Apex::Component* comp);
		bool DrawTreeComp(Apex::Component* obj);

		void HandleSelection(Apex::Component* comp);

	private:
		Apex::UserInterface::IGUI& m_gui;
		Apex::Rendering::Scene& m_scene;

		Apex::Data::Object* m_selectedObj = nullptr;
		Apex::Component* m_selectedComp = nullptr;
	};
}

#endif // !COMPONENTS_VIEWER
