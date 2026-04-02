#ifndef HUD
#define HUD

#include "UI.h"
#include "IDrawList.h"
#include "ResourceHandle.h"
#include "ResourceManager.h"
#include "Texture.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <filesystem>

using LibMath::Vector2;
using Apex::Resources::ResourceHandle;
using Apex::Resources::ResourceManager;

namespace fs = std::filesystem;

namespace Apex::UserInterface
{
	enum class UIAnchor
	{
		TopLeft, TopCenter, TopRight,
		MiddleLeft, Center, MiddleRight,
		BottomLeft, BottomCenter, BottomRight
	};

	enum class UIWidgetType { Text, Image, Button, ProgressBar, Panel };

	struct UIColor { float m_r = 1, m_g = 1, m_b = 1, m_a = 1; };

	inline uint32_t UIColorPack(UIColor color)
	{
		auto b = [](float v) { return static_cast<uint32_t>(v * 255.f) & 0xFF; };
		return b(color.m_a) << 24 | b(color.m_b) << 16 | b(color.m_g) << 8 | b(color.m_r);
	}

	struct UIDrawContext
	{
		IDrawList*  m_drawList = nullptr;
		Vector2     m_screenSize;
		bool        m_isEditor = false;
		bool        m_selected = false;
	};

	class UIElement
	{
	public:
		UIElement() = default;
		virtual ~UIElement() = default;

		virtual UIWidgetType GetType() const = 0;
		virtual void Draw(const UIDrawContext& context) const = 0;

		static Vector2 ApplyAnchor(Vector2 pos, UIAnchor anchor, Vector2 screen);
		Vector2 GetScreenPos(Vector2 screen) const;
		Vector2 GetScreenSize(Vector2 screen) const;

		std::string m_name = "Widget";
		Vector2     m_position = { 0.5f, 0.5f };
		Vector2     m_size = { 0.2f, 0.05f };
		UIAnchor    m_anchor = UIAnchor::TopLeft;
		UIColor     m_color = { 1, 1, 1, 1 };
		bool        m_visible = true;
	};

    class UIText : public UIElement
    {
    public:
        UIWidgetType GetType() const override { return UIWidgetType::Text; }
        void Draw(const UIDrawContext& context) const override;

        std::string m_text = "Text";
        FontID      m_font = FontID::Default;
        float       m_fontSize = 24.f;
    };

    class UIImage : public UIElement
    {
    public:
        UIWidgetType GetType() const override { return UIWidgetType::Image; }
        void Draw(const UIDrawContext& context) const override;
        void LoadTexture(ResourceManager& resourceManager);

        std::string                      m_texturePath;
        ResourceHandle<Texture>          m_texture;
    };

    class UIButton : public UIElement
    {
    public:
        UIWidgetType GetType() const override { return UIWidgetType::Button; }
        void Draw(const UIDrawContext& context) const override;

        std::string m_label = "Button";
        UIColor     m_colorHover = { 0.8f, 0.8f, 0.8f, 1.f };
        UIColor     m_colorPressed = { 0.6f, 0.6f, 0.6f, 1.f };
        UIColor     m_colorText = { 1.f,  1.f,  1.f,  1.f };
        float       m_fontSize = 20.f;

        std::function<void()> m_onClick;
    };

    class UIProgressBar : public UIElement
    {
    public:
        UIWidgetType GetType() const override { return UIWidgetType::ProgressBar; }
        void Draw(const UIDrawContext& context) const override;

        float   m_value = 0.5f;
        UIColor m_colorBg = { 0.2f, 0.2f, 0.2f, 1.f };
        UIColor m_colorFill = { 0.2f, 0.7f, 0.2f, 1.f };
        float   m_rounding = 4.f;
    };

    class UIPanel : public UIElement
    {
    public:
        UIWidgetType GetType() const override { return UIWidgetType::Panel; }
        void Draw(const UIDrawContext& context) const override;
        void AddChild(std::unique_ptr<UIElement> child);

        UIColor m_colorBg = { 0.1f, 0.1f, 0.1f, 0.8f };
        float   m_rounding = 6.f;
        std::vector<std::unique_ptr<UIElement>> m_children;
    };

    class UICanvas
    {
    public:
        UICanvas() = default;
        UICanvas(const UICanvas&) = delete;
        UICanvas& operator=(const UICanvas&) = delete;

        void AddElement(std::unique_ptr<UIElement> element);
        void RemoveElement(size_t index);
        void MoveElement(size_t from, size_t to);
        void Clear();

        UIElement* GetElement(size_t index);
        size_t GetElementCount() const { return m_elements.size(); }
        const std::vector<std::unique_ptr<UIElement>>& GetElements() const { return m_elements; }

        void Draw(const UIDrawContext& ctx) const;

        bool Save(const fs::path& path) const;
        bool Load(const fs::path& path, ResourceManager& resourceManager);

    private:
        std::vector<std::unique_ptr<UIElement>> m_elements;
    };

    class UIManager
    {
    public:
        UIManager() = delete;
        explicit UIManager(IGUI* gui, ResourceManager* resourceManager);
        UIManager(const UIManager&) = delete;
        UIManager& operator=(const UIManager&) = delete;

        UICanvas& GetCanvas() { return m_canvas; }

        bool LoadHUD(const fs::path& path);
        bool SaveHUD(const fs::path& path) const;

        void RenderHUD(IDrawList* backgroundDrawList);

    private:
        UICanvas       m_canvas;
        IGUI* m_gui = nullptr;
        ResourceManager* m_resourceManager = nullptr;
    };
}

#endif // !HUD
