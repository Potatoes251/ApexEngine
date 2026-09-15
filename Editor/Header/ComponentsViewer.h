#ifndef COMPONENTS_VIEWER
#define COMPONENTS_VIEWER

#include "UI.h"

#include "Scene.h"
#include "Object.h"
#include "Component.h"
#include "AssetPicker.h"
#include "ScriptPicker.h"
#include "ScriptComponent.h"
#include "ThumbnailRenderer.h"
#include "EditorIcon.h"
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"
#include "ResourceManager.h"
#include "ResourceHandle.h"
#include "Physic.h"
#include "LuaManager.h"

namespace Apex::Editor
{
	class ComponentsViewer
	{
	public:
		ComponentsViewer(Apex::UserInterface::IGUI* gui,
			Apex::Rendering::Scene* scene,
			Apex::Resources::ResourceManager* resourceManager,
			Apex::Scripting::LuaManager* lua);
		
		void SetSelectedObject(Apex::Data::Object* obj) { m_selectedObj = obj; }
		void SetScene(Apex::Rendering::Scene* scene) { m_scene = scene; }
		void ClearSelection();
		void SetThumbnailRenderer(ThumbnailRenderer* t);

		void Draw();

	private:
		void DrawHeader();
		void DrawComponentsList();

		void DrawAddComponentButton();
		void DrawAddComponentPopup();

		void DrawRenderComponentsMenu();
		void DrawPhysicsComponentsMenu();
		void DrawScriptComponentsMenu();
		void DrawLightComponentsMenu();
		void DrawCameraComponentsMenu();
		void DrawControllerComponentsMenu();
		void DrawMiscComponentsMenu();

		void DrawTransform(Apex::Data::Object* obj);
		void DrawComponent(Apex::Data::Object* obj, Apex::Component* comp);

		bool DrawVec3(const std::string& name, LibMath::Vector3& v, float min, float max);

		void HandleSelection(Apex::Component* comp);

		void DrawScriptComponent(Apex::Scripting::ScriptComponent* script);
		void DrawVar(ExposedVar& var, Apex::Data::Object* obj, Apex::Component* comp);

		bool DrawComponentEnum(ExposedVar var);
		bool DrawComponent(std::string const& name, LibMath::Vector3& data);
		bool DrawComponentColor3(ExposedVar var);
		bool DrawComponentColor4(ExposedVar var);
		void DrawComponent(std::string const& name, Resources::ResourceHandle<Model>* handle);
		void DrawComponent(std::string const& name, Resources::ResourceHandle<Rendering::Material>* material);
		void DrawComponent(std::string const& name, Resources::ResourceHandle<Texture>* handle);

		uint32_t ResolveTexturePreview(Resources::ResourceHandle<Texture>* handle) const;
		uint32_t ResolveMeshPreview(const std::string& path) const;
		uint32_t ResolveMaterialPreview(const std::string& path) const;
		void     DrawAssetPreview(uint32_t texID, const std::string& fallbackLabel, float size = 64.f);

		UserInterface::IGUI* m_gui;
		Rendering::Scene* m_scene = nullptr;
		Resources::ResourceManager* m_resourceManager;
		Resources::AssetPicker<Model> m_meshPicker;
		Resources::AssetPicker<Rendering::Material> m_materialPicker;
		Resources::AssetPicker<Texture> m_texturePicker;
		Resources::ScriptPicker m_scriptPicker;
		Scripting::LuaManager* m_luaManager;
		ThumbnailRenderer* m_thumbnailRenderer = nullptr;

		Apex::Data::Object* m_selectedObj = nullptr;
		Apex::Component* m_selectedComp = nullptr;

		Apex::Component* m_pendingDelete = nullptr;

		bool m_changed = false;

		LibMath::Vector3 m_cachedEuler;
		Apex::Data::Object* m_lastTransformObj = nullptr;

		static constexpr float LABEL_WIDTH = 200.f;
		static constexpr float ROW_PAD = 5.f;
		static constexpr float PREVIEW_SIZE = 64.f;
	};
}

#endif // !COMPONENTS_VIEWER
