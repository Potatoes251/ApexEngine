#ifndef MATERIAL_EDITOR
#define MATERIAL_EDITOR

#include "Material.h"

#include "UI.h"
#include "AssetPicker.h"

namespace Apex::Editor
{
	class MaterialEditor
	{
	public:
		MaterialEditor(UserInterface::IGUI* gui, Resources::ResourceManager* resourceManager)
			: m_gui(gui), m_resourceManager(resourceManager), m_texturePicker(gui, resourceManager) {}

		void Open(std::string path);
		void Close();

		void Draw();

		bool IsOpen() const { return m_open; }

		void SetThumbnailRenderer(ThumbnailRenderer* t);

	private:
        void DrawPreviewAndHeader();
        void DrawTint();
        void DrawTextureList();
        void SaveMaterial();
        void DrawTexture(Apex::Resources::ResourceHandle<Texture>* texture, int id);
        void DrawShaderPicker(std::string& out_shaderPath, bool& out_open, const std::string shaderType);

        Apex::UserInterface::IGUI* m_gui = nullptr;
        Apex::Resources::ResourceManager* m_resourceManager = nullptr;
        ThumbnailRenderer* m_thumbnails = nullptr;

        Apex::Resources::ResourceHandle<Apex::Rendering::Material> m_currentMat;
        Apex::Resources::AssetPicker<Texture> m_texturePicker;

		std::string m_computeShader;

		bool m_compShadPickOpen = false;
		bool m_open = false;
		bool m_wasReady = false;
	};
}

#endif // !MATERIAL_EDITOR

