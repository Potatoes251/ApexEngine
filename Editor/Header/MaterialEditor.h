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

	private:
		void DrawTexture(Resources::ResourceHandle<Texture>* texture, int id);
		void DrawShaderPicker(std::string& shaderPath, bool& open, std::string shaderType);

		Resources::AssetPicker<Texture> m_texturePicker;

		Resources::ResourceHandle<Rendering::Material> m_currentMat;
		UserInterface::IGUI* m_gui;
		Resources::ResourceManager* m_resourceManager;

		std::string m_computeShader;
		std::string m_vertexShader;
		std::string m_fragmentShader;

		bool m_vertShadPickOpen = false;
		bool m_fragShadPickOpen = false;
		bool m_compShadPickOpen = false;
		bool m_open = false;
		bool m_wasReady = false;
	};
}

#endif // !MATERIAL_EDITOR

