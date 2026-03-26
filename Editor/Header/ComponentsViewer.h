#ifndef COMPONENTS_VIEWER
#define COMPONENTS_VIEWER

#include "UI.h"

#include "Scene.h"
#include "Object.h"
#include "Component.h"
#include "AssetPicker.h"
#include "ScriptPicker.h"
#include "ScriptComponent.h"
#include "Texture.h"
#include "Mesh.h"
#include "ResourceManager.h"
#include "Physic.h"
#include "LuaManager.h"

namespace Apex::Editor
{
	class ComponentsViewer
	{
	public:
		ComponentsViewer(Apex::UserInterface::IGUI& gui,
			Apex::Rendering::Scene& scene, 
			Apex::Resources::ResourceManager& resourceManager,
			Apex::Physic::PhysicSystem& phys,
			Apex::Scripting::LuaManager& lua);
		
		void SetSelectedObject(Apex::Data::Object* obj) { m_selectedObj = obj; }

		void Draw();

		void DrawComponent(Apex::Component* comp);
		bool DrawTreeComp(Apex::Component* obj);

		void HandleSelection(Apex::Component* comp);

	private:
		void DrawComponentEnum(ExposedVar var);
		void DrawComponentVector3(ExposedVar var);
		void DrawComponentPhysMaterial(ExposedVar var);
		void DrawComponentMesh(ExposedVar var);
		void DrawComponentTexture(ExposedVar var);
		void DrawComponentScript(ExposedVar var);

		Apex::UserInterface::IGUI& m_gui;
		Apex::Rendering::Scene& m_scene;
		Apex::Resources::ResourceManager& m_resourceManager;
		Apex::Resources::AssetPicker<Mesh> m_meshPicker;
		Apex::Resources::AssetPicker<Texture> m_texturePicker;
		Apex::Resources::ScriptPicker m_scriptPicker;
		Apex::Physic::PhysicSystem& m_phys;
		Apex::Scripting::LuaManager& m_luaManager;

		Apex::Data::Object* m_selectedObj = nullptr;
		Apex::Component* m_selectedComp = nullptr;
	};
}

#endif // !COMPONENTS_VIEWER
