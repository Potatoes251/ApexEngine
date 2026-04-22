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
#include "Material.h"
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
		void ClearSelection();

		void Draw();

		void DrawTransform(Apex::Data::Object* obj);
		void DrawComponent(Apex::Data::Object* obj, Apex::Component* comp);

		bool DrawVec3(const std::string& name, LibMath::Vector3& v, float min, float max);

		void HandleSelection(Apex::Component* comp);

	private:
		void DrawScriptComponent(Apex::Scripting::ScriptComponent* script);
		void DrawVar(ExposedVar& var, Apex::Data::Object* obj, Apex::Component* comp);

		bool DrawComponentEnum(ExposedVar var);
		bool DrawComponentVector3(ExposedVar var);
		bool DrawComponentPhysMaterial(ExposedVar var);
		bool DrawComponentColor3(ExposedVar var);
		bool DrawComponentColor4(ExposedVar var);
		void DrawComponentMesh(ExposedVar var);
		void DrawComponentMat(ExposedVar var);
		void DrawComponentTexture(ExposedVar var);

		Apex::UserInterface::IGUI& m_gui;
		Apex::Rendering::Scene& m_scene;
		Apex::Resources::ResourceManager& m_resourceManager;
		Apex::Resources::AssetPicker<Mesh> m_meshPicker;
		Apex::Resources::AssetPicker<Rendering::Material> m_materialPicker;
		Apex::Resources::AssetPicker<Texture> m_texturePicker;
		Apex::Resources::ScriptPicker m_scriptPicker;
		Apex::Physic::PhysicSystem& m_phys;
		Apex::Scripting::LuaManager& m_luaManager;

		Apex::Data::Object* m_selectedObj = nullptr;
		Apex::Component* m_selectedComp = nullptr;

		Apex::Component* m_pendingDelete = nullptr;

		bool m_changed = false;

		LibMath::Vector3 m_cachedEuler;
		Apex::Data::Object* m_lastTransformObj = nullptr;
	};
}

#endif // !COMPONENTS_VIEWER
