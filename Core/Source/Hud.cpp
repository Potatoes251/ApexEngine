#include "Hud.h"
#include "SerializationParser.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

using namespace Apex::UserInterface;
using namespace Apex::Serialization;

Vector2 UIElement::ApplyAnchor(Vector2 pos, UIAnchor anchor, Vector2 screen)
{
    float anchorX = 0.f, anchorY = 0.f;
    switch (anchor)
    {
    case UIAnchor::TopCenter:    anchorX = 0.5f; anchorY = 0.f;  break;
    case UIAnchor::TopRight:     anchorX = 1.f;  anchorY = 0.f;  break;
    case UIAnchor::MiddleLeft:   anchorX = 0.f;  anchorY = 0.5f; break;
    case UIAnchor::Center:       anchorX = 0.5f; anchorY = 0.5f; break;
    case UIAnchor::MiddleRight:  anchorX = 1.f;  anchorY = 0.5f; break;
    case UIAnchor::BottomLeft:   anchorX = 0.f;  anchorY = 1.f;  break;
    case UIAnchor::BottomCenter: anchorX = 0.5f; anchorY = 1.f;  break;
    case UIAnchor::BottomRight:  anchorX = 1.f;  anchorY = 1.f;  break;
    default: break;
    }
    return { anchorX * screen[0] + pos[0] * screen[0],
             anchorY * screen[1] + pos[1] * screen[1] };
}

Vector2 UIElement::GetScreenPos(Vector2 screen) const
{
    return ApplyAnchor(m_position, m_anchor, screen);
}

Vector2 UIElement::GetScreenSize(Vector2 screen) const
{
    return { m_size[0] * screen[0], m_size[1] * screen[1] };
}

void UIText::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;
    Vector2 pos = GetScreenPos(context.m_screenSize);
    context.m_drawList->DrawTextEx(pos, UIColorPack(m_color), m_text, m_fontSize);

    if (context.m_isEditor && context.m_selected)
    {
        Vector2 size = { static_cast<float>(m_text.size()) * m_fontSize * 0.6f, m_fontSize };
        context.m_drawList->DrawRect(pos, { pos[0] + size[0], pos[1] + size[1] }, 0xFF44AAFF, 0.f, 1.5f);
    }
}

void UIImage::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;
    Vector2 pos = GetScreenPos(context.m_screenSize);
    Vector2 sz = GetScreenSize(context.m_screenSize);
    Vector2 max = { pos[0] + sz[0], pos[1] + sz[1] };

    if (m_texture.IsReady())
    {
        uint32_t texID = m_texture->GetId();
        context.m_drawList->DrawRectFilled(pos, max, UIColorPack(m_color));
    }
    else
    {
        context.m_drawList->DrawRectFilled(pos, max, 0x884444AA);
        context.m_drawList->DrawTextEx({ pos[0] + 4.f, pos[1] + 4.f }, 0xFFAAAAAA, "Image", 14.f);
    }

    if (context.m_isEditor && context.m_selected)
        context.m_drawList->DrawRect(pos, max, 0xFF44AAFF, 0.f, 1.5f);
}

void UIImage::LoadTexture(ResourceManager& rm)
{
    if (!m_texturePath.empty())
        m_texture = rm.CreateAsync<Texture>(m_texturePath);
}

void UIButton::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;
    Vector2 pos = GetScreenPos(context.m_screenSize);
    Vector2 size = GetScreenSize(context.m_screenSize);
    Vector2 max = { pos[0] + size[0], pos[1] + size[1] };

    context.m_drawList->DrawRectFilled(pos, max, UIColorPack(m_color), 4.f);

    Vector2 center = { pos[0] + size[0] * 0.5f, pos[1] + size[1] * 0.5f - m_fontSize * 0.5f };
    context.m_drawList->DrawTextEx(center, UIColorPack(m_colorText), m_label, m_fontSize);

    if (context.m_isEditor && context.m_selected)
        context.m_drawList->DrawRect(pos, max, 0xFF44AAFF, 4.f, 1.5f);
}

void UIProgressBar::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;
    Vector2 pos = GetScreenPos(context.m_screenSize);
    Vector2 sz = GetScreenSize(context.m_screenSize);
    Vector2 max = { pos[0] + sz[0], pos[1] + sz[1] };

    context.m_drawList->DrawRectFilled(pos, max, UIColorPack(m_colorBg), m_rounding);

    float fill = std::max(0.f, std::min(1.f, m_value));
    Vector2 fillMax = { pos[0] + sz[0] * fill, max[1] };
    if (fill > 0.f)
        context.m_drawList->DrawRectFilled(pos, fillMax, UIColorPack(m_colorFill), m_rounding);

    if (context.m_isEditor && context.m_selected)
        context.m_drawList->DrawRect(pos, max, 0xFF44AAFF, m_rounding, 1.5f);
}

void UIPanel::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;
    Vector2 pos = GetScreenPos(context.m_screenSize);
    Vector2 sz = GetScreenSize(context.m_screenSize);
    Vector2 max = { pos[0] + sz[0], pos[1] + sz[1] };

    context.m_drawList->DrawRectFilled(pos, max, UIColorPack(m_colorBg), m_rounding);

    UIDrawContext childCtx = context;
    for (const auto& child : m_children)
        if (child) child->Draw(childCtx);

    if (context.m_isEditor && context.m_selected)
        context.m_drawList->DrawRect(pos, max, 0xFF44AAFF, m_rounding, 1.5f);
}

void UIPanel::AddChild(std::unique_ptr<UIElement> child)
{
    m_children.push_back(std::move(child));
}

void UICanvas::AddElement(std::unique_ptr<UIElement> element)
{
    m_elements.push_back(std::move(element));
}

void UICanvas::RemoveElement(size_t index)
{
    if (index < m_elements.size())
        m_elements.erase(m_elements.begin() + static_cast<ptrdiff_t>(index));
}

void UICanvas::MoveElement(size_t from, size_t to)
{
    if (from >= m_elements.size() || to >= m_elements.size()) return;
    if (from == to) return;
    auto element = std::move(m_elements[from]);
    m_elements.erase(m_elements.begin() + from);
    m_elements.insert(m_elements.begin() + to, std::move(element));
}

void UICanvas::Clear()
{
    m_elements.clear();
}

UIElement* UICanvas::GetElement(size_t index)
{
    return (index < m_elements.size()) ? m_elements[index].get() : nullptr;
}

void UICanvas::Draw(const UIDrawContext& context) const
{
    for (const auto& e : m_elements)
        if (e) e->Draw(context);
}

static const char* AnchorName(UIAnchor anchor)
{
    switch (anchor)
    {
    case UIAnchor::TopLeft:      return "TopLeft";
    case UIAnchor::TopCenter:    return "TopCenter";
    case UIAnchor::TopRight:     return "TopRight";
    case UIAnchor::MiddleLeft:   return "MiddleLeft";
    case UIAnchor::Center:       return "Center";
    case UIAnchor::MiddleRight:  return "MiddleRight";
    case UIAnchor::BottomLeft:   return "BottomLeft";
    case UIAnchor::BottomCenter: return "BottomCenter";
    case UIAnchor::BottomRight:  return "BottomRight";
    default:                     return "TopLeft";
    }
}

static UIAnchor AnchorFromName(const std::string& string)
{
    if (string == "TopCenter")    return UIAnchor::TopCenter;
    if (string == "TopRight")     return UIAnchor::TopRight;
    if (string == "MiddleLeft")   return UIAnchor::MiddleLeft;
    if (string == "Center")       return UIAnchor::Center;
    if (string == "MiddleRight")  return UIAnchor::MiddleRight;
    if (string == "BottomLeft")   return UIAnchor::BottomLeft;
    if (string == "BottomCenter") return UIAnchor::BottomCenter;
    if (string == "BottomRight")  return UIAnchor::BottomRight;
    return UIAnchor::TopLeft;
}

static void WriteColor(std::ofstream& file, const char* key, UIColor color, const char* indent)
{
    file << indent << "\"" << key << "\": [ "
        << color.m_r << ", " << color.m_g << ", " << color.m_b << ", " << color.m_a << " ]";
}

static void WriteVec2(std::ofstream& file, const char* key, Vector2 vector, const char* indent)
{
    file << indent << "\"" << key << "\": [ " << vector[0] << ", " << vector[1] << " ]";
}

static UIColor ParseColor(SerialParser& parser)
{
    parser.Expect('[');
    UIColor color;
    color.m_r = parser.ParseFloat(); parser.Expect(',');
    color.m_g = parser.ParseFloat(); parser.Expect(',');
    color.m_b = parser.ParseFloat(); parser.Expect(',');
    color.m_a = parser.ParseFloat();
    parser.Expect(']');
    return color;
}

static Vector2 ParseVec2(SerialParser& parser)
{
    parser.Expect('[');
    float x = parser.ParseFloat(); parser.Expect(',');
    float y = parser.ParseFloat();
    parser.Expect(']');
    return { x, y };
}

static void WriteElement(std::ofstream& file, const UIElement& element, const std::string& indent)
{
    file << indent << "{\n";
    // "type" is always first so the parser can dispatch without lookahead
    switch (element.GetType())
    {
        case UIWidgetType::Text:        file << indent << "  \"type\": \"Text\",\n"; break;
        case UIWidgetType::Image:       file << indent << "  \"type\": \"Image\",\n"; break;
        case UIWidgetType::Button:      file << indent << "  \"type\": \"Button\",\n"; break;
        case UIWidgetType::ProgressBar: file << indent << "  \"type\": \"ProgressBar\",\n"; break;
        case UIWidgetType::Panel:       file << indent << "  \"type\": \"Panel\",\n"; break;
    }
    file << indent << "  \"name\": \"" << element.m_name << "\",\n";
    file << indent << "  \"anchor\": \"" << AnchorName(element.m_anchor) << "\",\n";
    file << indent << "  \"visible\": " << (element.m_visible ? "true" : "false") << ",\n";
    WriteVec2(file, "position", element.m_position, (indent + "  ").c_str()); file << ",\n";
    WriteVec2(file, "size", element.m_size, (indent + "  ").c_str()); file << ",\n";
    WriteColor(file, "color", element.m_color, (indent + "  ").c_str()); file << ",\n";

    switch (element.GetType())
    {
        case UIWidgetType::Text:
        {
            const auto& text = static_cast<const UIText&>(element);
            file << indent << "  \"text\": \"" << text.m_text << "\",\n";
            file << indent << "  \"fontSize\": " << text.m_fontSize << "\n";
            break;
        }
        case UIWidgetType::Image:
        {
            const auto& image = static_cast<const UIImage&>(element);
            file << indent << "  \"texture\": \"" << image.m_texturePath << "\"\n";
            break;
        }
        case UIWidgetType::Button:
        {
            const auto& button = static_cast<const UIButton&>(element);
            file << indent << "  \"label\": \"" << button.m_label << "\",\n";
            file << indent << "  \"fontSize\": " << button.m_fontSize << ",\n";
            WriteColor(file, "colorHover", button.m_colorHover, (indent + "  ").c_str()); file << ",\n";
            WriteColor(file, "colorPressed", button.m_colorPressed, (indent + "  ").c_str()); file << ",\n";
            WriteColor(file, "colorText", button.m_colorText, (indent + "  ").c_str()); file << "\n";
            break;
        }
        case UIWidgetType::ProgressBar:
        {
            const auto& progressBar = static_cast<const UIProgressBar&>(element);
            file << indent << "  \"value\": " << progressBar.m_value << ",\n";
            file << indent << "  \"rounding\": " << progressBar.m_rounding << ",\n";
            WriteColor(file, "colorBg", progressBar.m_colorBg, (indent + "  ").c_str()); file << ",\n";
            WriteColor(file, "colorFill", progressBar.m_colorFill, (indent + "  ").c_str()); file << "\n";
            break;
        }
        case UIWidgetType::Panel:
        {
            const auto& panel = static_cast<const UIPanel&>(element);
            file << indent << "  \"rounding\": " << panel.m_rounding << ",\n";
            WriteColor(file, "colorBg", panel.m_colorBg, (indent + "  ").c_str()); file << ",\n";
            file << indent << "  \"children\": [\n";
            bool first = true;
            for (const auto& child : panel.m_children)
            {
                if (!first) file << ",\n";
                first = false;
                WriteElement(file, *child, indent + "    ");
            }
            file << "\n" << indent << "  ]\n";
            break;
        }
    }
    file << indent << "}";
}


static std::unique_ptr<UIElement> ParseElement(SerialParser& parser, ResourceManager* resourceManager)
{
    parser.Expect('{');
    parser.ParseString();
    parser.Expect(':');
    std::string type = parser.ParseString();
    if (parser.Peek(',')) parser.Expect(',');

    std::unique_ptr<UIElement> element;
    if (type == "Text")             element = std::make_unique<UIText>();
    else if (type == "Image")       element = std::make_unique<UIImage>();
    else if (type == "Button")      element = std::make_unique<UIButton>();
    else if (type == "ProgressBar") element = std::make_unique<UIProgressBar>();
    else if (type == "Panel")       element = std::make_unique<UIPanel>();
    else 
    { 
        parser.Expect('}'); 
        return nullptr; 
    }

    while (!parser.Peek('}'))
    {
        std::string key = parser.ParseString();
        parser.Expect(':');

        // Base fields
        if (key == "name")          element->m_name = parser.ParseString();
        else if (key == "anchor")   element->m_anchor = AnchorFromName(parser.ParseString());
        else if (key == "visible")  element->m_visible = parser.ParseBool();
        else if (key == "position") element->m_position = ParseVec2(parser);
        else if (key == "size")     element->m_size = ParseVec2(parser);
        else if (key == "color")    element->m_color = ParseColor(parser);

        // Type-specific fields
        else if (auto* text = dynamic_cast<UIText*>(element.get()))
        {
            if (key == "text")     text->m_text = parser.ParseString();
            else if (key == "fontSize") text->m_fontSize = parser.ParseFloat();
        }
        else if (auto* image = dynamic_cast<UIImage*>(element.get()))
        {
            if (key == "texture")
            {
                image->m_texturePath = parser.ParseString();
                if (resourceManager && !image->m_texturePath.empty())
                    image->LoadTexture(*resourceManager);
            }
        }
        else if (auto* button = dynamic_cast<UIButton*>(element.get()))
        {
            if (key == "label")             button->m_label = parser.ParseString();
            else if (key == "fontSize")     button->m_fontSize = parser.ParseFloat();
            else if (key == "colorHover")   button->m_colorHover = ParseColor(parser);
            else if (key == "colorPressed") button->m_colorPressed = ParseColor(parser);
            else if (key == "colorText")    button->m_colorText = ParseColor(parser);
        }
        else if (auto* progressBar = dynamic_cast<UIProgressBar*>(element.get()))
        {
            if (key == "value")          progressBar->m_value = parser.ParseFloat();
            else if (key == "rounding")  progressBar->m_rounding = parser.ParseFloat();
            else if (key == "colorBg")   progressBar->m_colorBg = ParseColor(parser);
            else if (key == "colorFill") progressBar->m_colorFill = ParseColor(parser);
        }
        else if (auto* panel = dynamic_cast<UIPanel*>(element.get()))
        {
            if (key == "rounding") panel->m_rounding = parser.ParseFloat();
            else if (key == "colorBg") panel->m_colorBg = ParseColor(parser);
            else if (key == "children")
            {
                parser.Expect('[');
                while (!parser.Peek(']'))
                {
                    auto child = ParseElement(parser, resourceManager);
                    if (child) panel->AddChild(std::move(child));
                    if (parser.Peek(',')) parser.Expect(',');
                }
                parser.Expect(']');
            }
        }

        if (parser.Peek(',')) parser.Expect(',');
    }

    parser.Expect('}');
    return element;
}


bool UICanvas::Save(const fs::path& path) const
{
    std::ofstream file(path);
    if (!file) return false;

    file << std::fixed << std::setprecision(6);
    file << "{\n  \"widgets\": [\n";

    bool first = true;
    for (const auto& e : m_elements)
    {
        if (!first) file << ",\n";
        first = false;
        WriteElement(file, *e, "    ");
    }

    file << "\n  ]\n}\n";
    return true;
}

bool UICanvas::Load(const fs::path& path, ResourceManager& rm)
{
    std::ifstream file(path);
    if (!file) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    SerialParser parser(text);
    parser.Expect('{');
    parser.ParseString();
    parser.Expect(':');
    parser.Expect('[');

    m_elements.clear();

    while (!parser.Peek(']'))
    {
        auto element = ParseElement(parser, &rm);
        if (element) m_elements.push_back(std::move(element));
        if (parser.Peek(',')) parser.Expect(',');
    }

    parser.Expect(']');
    parser.Expect('}');
    return true;
}

UIManager::UIManager(IGUI* gui, ResourceManager* rm)
    : m_gui(gui), m_resourceManager(rm)
{
}


bool UIManager::LoadHUD(const fs::path& path)
{
    if (!m_resourceManager) return false;
    return m_canvas.Load(path, *m_resourceManager);
}

bool UIManager::SaveHUD(const fs::path& path) const
{
    return m_canvas.Save(path);
}

void UIManager::RenderHUD(IDrawList* backgroundDrawList)
{
    if (!m_gui || !backgroundDrawList) return;

    UIDrawContext context;
    context.m_drawList = backgroundDrawList;
    context.m_screenSize = m_gui->GetScreenSize();
    context.m_isEditor = false;
    context.m_selected = false;

    m_canvas.Draw(context);
}
