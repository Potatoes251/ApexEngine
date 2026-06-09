#ifndef HUD
#define HUD

#include "UI.h"
#include "IDrawList.h"
#include "ResourceHandle.h"
#include "ResourceManager.h"
#include "Texture.h"
#include <lua.hpp>

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <filesystem>

using Apex::Resources::ResourceHandle;
using Apex::Resources::ResourceManager;

namespace Fs = std::filesystem;

namespace Apex::UserInterface
{
	enum class UIAnchor
	{
		TopLeft, TopCenter, TopRight,
		MiddleLeft, Center, MiddleRight,
		BottomLeft, BottomCenter, BottomRight
	};

	enum class UIWidgetType { Text, Image, Button, ProgressBar, Panel };

    enum class TextHAlign { Left, Center, Right };
    enum class TextVAlign { Top, Middle, Bottom };

	struct UIColor { float m_r = 1, m_g = 1, m_b = 1, m_a = 1; };

	inline uint32_t UIColorPack(UIColor color)
	{
		auto b = [](float v) { return static_cast<uint32_t>(v * 255.f) & 0xFF; };
		return b(color.m_a) << 24 | b(color.m_b) << 16 | b(color.m_g) << 8 | b(color.m_r);
	}

	struct UIDrawContext
	{
		IDrawList*  m_drawList = nullptr;
		LibMath::Vector2     m_screenSize;
        LibMath::Vector2     m_origin = { 0.f, 0.f };
        Apex::Rendering::IRHI* m_rhi = nullptr;
		bool        m_isEditor = false;
		bool        m_selected = false;

	};

	struct UIElement
	{
	public:
		UIElement() = default;
		virtual ~UIElement() = default;

		virtual UIWidgetType GetType() const = 0;
		virtual void Draw(const UIDrawContext& context) const = 0;

		static LibMath::Vector2 ApplyAnchor(LibMath::Vector2 pos, UIAnchor anchor, LibMath::Vector2 screen);
		LibMath::Vector2 GetScreenPos(const UIDrawContext& context) const;
		LibMath::Vector2 GetScreenSize(const UIDrawContext& context) const;

		std::string m_name = "Widget";
		LibMath::Vector2     m_position = { 0.5f, 0.5f };
		LibMath::Vector2     m_size = { 0.2f, 0.05f };
		UIAnchor    m_anchor = UIAnchor::TopLeft;
		UIColor     m_color = { 1, 1, 1, 1 };
		bool        m_visible = true;
	};

    struct UIText : public UIElement
    {
    public:
        UIWidgetType GetType() const override { return UIWidgetType::Text; }
        void Draw(const UIDrawContext& context) const override;

        std::string m_text = "Text";
        FontID      m_font = FontID::Default;
        float       m_fontSize = 24.f;
        TextHAlign  m_hAlign = TextHAlign::Left;
        TextVAlign  m_vAlign = TextVAlign::Top;
        UIColor     m_textColor = { 1.f, 1.f, 1.f, 1.f };
    };

    struct UIImage : public UIElement
    {
    public:
        UIWidgetType GetType() const override { return UIWidgetType::Image; }
        void Draw(const UIDrawContext& context) const override;
        void LoadTexture(ResourceManager& resourceManager);

        std::string                      m_texturePath;
        ResourceHandle<Texture>          m_texture;
        mutable uint32_t                 m_glTexID = 0;
    };

    struct UIButton : public UIElement
    {
    public:
        UIButton()
        {
            m_labelText.m_position = { 0.f, 0.f };
            m_labelText.m_size = { 1.f, 1.f };
            m_labelText.m_anchor = UIAnchor::TopLeft;
            m_labelText.m_hAlign = TextHAlign::Center;
            m_labelText.m_vAlign = TextVAlign::Middle;
            m_labelText.m_text = "Button";
            m_labelText.m_fontSize = 20.f;
            m_labelText.m_textColor = { 1.f, 1.f, 1.f, 1.f };
            m_labelText.m_visible = true;
        }

        UIWidgetType GetType() const override { return UIWidgetType::Button; }
        void Draw(const UIDrawContext& context) const override;

        UIText      m_labelText;
        UIColor     m_colorHover = { 0.8f, 0.8f, 0.8f, 1.f };
        UIColor     m_colorPressed = { 0.6f, 0.6f, 0.6f, 1.f };

        //LUA
        lua_State*  m_luaState;
        int         m_luaFuncRef = LUA_NOREF;

        std::function<void()> m_onClick;
    };

    struct UIProgressBar : public UIElement
    {
    public:
        UIWidgetType GetType() const override { return UIWidgetType::ProgressBar; }
        void Draw(const UIDrawContext& context) const override;

        float   m_value = 0.5f;
        UIColor m_colorBg = { 0.2f, 0.2f, 0.2f, 1.f };
        UIColor m_colorFill = { 0.2f, 0.7f, 0.2f, 1.f };
        float   m_rounding = 4.f;
    };

    struct UIPanel : public UIElement
    {
    public:
        UIWidgetType GetType() const override { return UIWidgetType::Panel; }
        void Draw(const UIDrawContext& context) const override;
        void AddChild(std::unique_ptr<UIElement> child);
        std::unique_ptr<UIElement> ExtractChild(size_t index);

        UIColor m_colorBg = { 0.1f, 0.1f, 0.1f, 0.8f };
        float   m_rounding = 6.f;
        std::vector<std::unique_ptr<UIElement>> m_children;
    };

    struct UICanvas
    {
    public:
        UICanvas() = default;
        UICanvas(const UICanvas&) = delete;
        UICanvas& operator=(const UICanvas&) = delete;

        void AddElement(std::unique_ptr<UIElement> element);
        void RemoveElement(size_t index);
        void MoveElement(size_t from, size_t to);
        void Clear();

        UIElement* GetElement(size_t index) const;
        size_t GetElementCount() const { return m_elements.size(); }
        const std::vector<std::unique_ptr<UIElement>>& GetElements() const { return m_elements; }
        std::unique_ptr<UIElement> ExtractElement(size_t index);

        void Draw(const UIDrawContext& context) const;
        void PrepareForRender(Apex::Rendering::IRHI* rhi);

        bool Save(const Fs::path& path) const;
        bool Load(const Fs::path& path, ResourceManager& resourceManager);

    private:
        std::vector<std::unique_ptr<UIElement>> m_elements;
    };

    struct HUDInstance
    {
        HUDInstance() = default;
        HUDInstance(const HUDInstance&) = delete;
        HUDInstance& operator=(const HUDInstance&) = delete;

        std::string  m_name;       
        UICanvas     m_canvas;
        bool         m_visible = false;
    };

    class UIManager
    {
    public:
        UIManager() = delete;
        explicit UIManager(IGUI* gui, ResourceManager* resourceManager, Apex::Rendering::IRHI* rhi = nullptr);
        UIManager(const UIManager&) = delete;
        UIManager& operator=(const UIManager&) = delete;

        // Multi-HUD API (used by Lua scripts)
        // Scan Assets/ for all .hud files and register their names.
        // Called once when entering Play.
        void ScanHUDs(const Fs::path& assetsRoot = "Assets/");

        // Create (load) a HUD by name (filename stem, e.g. "GameHUD").
        // Returns a handle >= 1 on success, 0 on failure.
        // Calling CreateHUD with the same name returns the existing handle.
        int  CreateHUD(const std::string& name);

        // Destroy a HUD instance and free its canvas.
        void DestroyHUD(int handle);

        // Show / Hide a specific HUD instance.
        void ShowHUD(int handle);
        void HideHUD(int handle);
        bool IsHUDVisible(int handle) const;

        // Prepare + Render all visible HUD instances.
        void RenderHUD(IDrawList* drawList,
            LibMath::Vector2 viewportPos = { 0.f, 0.f },
            LibMath::Vector2 viewportSize = { 0.f, 0.f });

        // Clears all instances (called on Stop).
        void ClearAll();

        HUDInstance* GetInstance(int handle);
        const HUDInstance* GetInstance(int handle) const;
        ResourceManager* GetResourceManager() const { return m_resourceManager; }

    private:
        IGUI* m_gui = nullptr;
        ResourceManager* m_resourceManager = nullptr;
        Apex::Rendering::IRHI* m_rhi = nullptr;

        std::vector<std::unique_ptr<HUDInstance>> m_instances;
        std::unordered_map<std::string, Fs::path>  m_registry;
    };
}

#endif // !HUD
