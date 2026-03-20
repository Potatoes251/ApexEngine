#ifndef COMPONENTS_VIEWER
#define COMPONENTS_VIEWER

#include "UI.h"

#include "Scene.h"
#include "Object.h"
#include "Component.h"
#include "AssetPicker.h"
#include "Texture.h"
#include "Mesh.h"
#include "ResourceManager.h"

namespace Apex::Editor
{
	class ComponentsViewer
	{
	public:
		ComponentsViewer(Apex::UserInterface::IGUI& gui,
			Apex::Rendering::Scene& scene, 
			Apex::Resources::ResourceManager& resourceManager);
		
		void SetSelectedObject(Apex::Data::Object* obj) { m_selectedObj = obj; }

		void Draw();

		void DrawComponent(Apex::Component* comp);
		bool DrawTreeComp(Apex::Component* obj);

		void HandleSelection(Apex::Component* comp);

	private:
		Apex::UserInterface::IGUI& m_gui;
		Apex::Rendering::Scene& m_scene;
		Apex::Resources::ResourceManager& m_resourceManager;
		Apex::Resources::AssetPicker<Mesh> m_meshPicker;
		Apex::Resources::AssetPicker<Texture> m_texturePicker;

		Apex::Data::Object* m_selectedObj = nullptr;
		Apex::Component* m_selectedComp = nullptr;
	};
}

#endif // !COMPONENTS_VIEWER
