#include "Hud.h"
#include "SerializationParser.h"
#include "InputSystem.h"

#include "Log.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

using Apex::Input::InputSystem;

using namespace Apex::UserInterface;
using namespace Apex::Serialization;

LibMath::Vector2 UIElement::ApplyAnchor(LibMath::Vector2 pos, UIAnchor anchor, LibMath::Vector2 screen)
{
    float anchorX = 0.f, anchorY = 0.f;
    switch (anchor)
    {
    case UIAnchor::TopCenter:    
        anchorX = 0.5f; 
        anchorY = 0.f;  
        break;
    case UIAnchor::TopRight:     
        anchorX = 1.f;  
        anchorY = 0.f;  
        break;
    case UIAnchor::MiddleLeft:   
        anchorX = 0.f;  
        anchorY = 0.5f; 
        break;
    case UIAnchor::Center:       
        anchorX = 0.5f; 
        anchorY = 0.5f; 
        break;
    case UIAnchor::MiddleRight:  
        anchorX = 1.f;  
        anchorY = 0.5f; 
        break;
    case UIAnchor::BottomLeft:   
        anchorX = 0.f;  
        anchorY = 1.f;  
        break;
    case UIAnchor::BottomCenter: 
        anchorX = 0.5f; 
        anchorY = 1.f;  
        break;
    case UIAnchor::BottomRight:  
        anchorX = 1.f;  
        anchorY = 1.f;  
        break;
    default: break;
    }
    return { anchorX * screen[0] + pos[0] * screen[0],
             anchorY * screen[1] + pos[1] * screen[1] };
}

LibMath::Vector2 UIElement::GetScreenPos(const UIDrawContext& context) const
{
    LibMath::Vector2 position = ApplyAnchor(m_position, m_anchor, context.m_screenSize);
    return { position[0] + context.m_origin[0], position[1] + context.m_origin[1] };
}

LibMath::Vector2 UIElement::GetScreenSize(const UIDrawContext& context) const
{
    return { m_size[0] * context.m_screenSize[0], m_size[1] * context.m_screenSize[1] };
}

void UIText::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;

    LibMath::Vector2 origin = GetScreenPos(context);
    LibMath::Vector2 bounds = GetScreenSize(context);

    LibMath::Vector2 textSize = context.m_drawList->CalculateTextSizeEx(m_text, m_fontSize);

    // Horizontal alignment within the widget bounds
    float x = origin[0];
    if (m_hAlign == TextHAlign::Center) x += (bounds[0] - textSize[0]) * 0.5f;
    else if (m_hAlign == TextHAlign::Right)  x += bounds[0] - textSize[0];

    // Vertical alignment
    float y = origin[1];
    if (m_vAlign == TextVAlign::Middle) y += (bounds[1] - textSize[1]) * 0.5f;
    else if (m_vAlign == TextVAlign::Bottom) y += bounds[1] - textSize[1];

    LibMath::Vector2 drawPos = { x, y };
    context.m_drawList->DrawTextEx(drawPos, UIColorPack(m_textColor), m_text, m_fontSize);

    if (context.m_isEditor && context.m_selected)
        context.m_drawList->DrawRect(origin, { origin[0] + bounds[0], origin[1] + bounds[1] }, 0xFF44AAFF, 0.f, 1.5f);
}

void UIImage::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;
    LibMath::Vector2 pos = GetScreenPos(context);
    LibMath::Vector2 size = GetScreenSize(context);
    LibMath::Vector2 max = { pos[0] + size[0], pos[1] + size[1] };

    if (m_glTexID != 0)
    {
        uint32_t tint = UIColorPack(m_color);
        context.m_drawList->DrawImage(m_glTexID, pos, max,
            { 0.f, 1.f }, { 1.f, 0.f }, tint);
    }
    else
    {
        context.m_drawList->DrawRectFilled(pos, max, 0x884444AA);
        context.m_drawList->DrawTextEx({ pos[0] + 4.f, pos[1] + 4.f }, 0xFFAAAAAA,
            m_texturePath.empty() ? "Image" : "Loading...", 14.f);
    }

    if (context.m_isEditor && context.m_selected)
        context.m_drawList->DrawRect(pos, max, 0xFF44AAFF, 0.f, 1.5f);
}

void UIImage::LoadTexture(ResourceManager& resourceManager)
{
    if (!m_texturePath.empty())
    {
        m_glTexID = 0;
        m_texture = resourceManager.CreateAsync<Texture>(m_texturePath);
    }
}

void UIButton::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;
    LibMath::Vector2 pos = GetScreenPos(context);
    LibMath::Vector2 size = GetScreenSize(context);
    LibMath::Vector2 max = { pos[0] + size[0], pos[1] + size[1] };

    context.m_drawList->DrawRectFilled(pos, max, UIColorPack(m_color), 4.f);

    UIDrawContext labelContext = context;
    labelContext.m_origin = pos;
    labelContext.m_screenSize = size;
    labelContext.m_isEditor = false;
    m_labelText.Draw(labelContext);

    if (context.m_isEditor && context.m_selected)
        context.m_drawList->DrawRect(pos, max, 0xFF44AAFF, 4.f, 1.5f);

    InputSystem&    input = InputSystem::Get();
    Vector2         inputPos = input.GetMousePos();

    if (input.IsMouseButtonDown(Input::MouseButton::Left) &&
        inputPos[0] >= pos[0] &&
        inputPos[0] <= max[0] &&
        inputPos[1] >= pos[1] &&
        inputPos[1] <= max[1] &&
        m_onClick &&
        m_visible)                      //maybe to change for a system that allows HUDs to be closed and it's elements be invisible. 
    {
        m_onClick();
    }
}

void UIProgressBar::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;
    LibMath::Vector2 pos = GetScreenPos(context);
    LibMath::Vector2 size = GetScreenSize(context);
    LibMath::Vector2 max = { pos[0] + size[0], pos[1] + size[1] };

    context.m_drawList->DrawRectFilled(pos, max, UIColorPack(m_colorBg), m_rounding);

    float fill = std::max(0.f, std::min(1.f, m_value));
    LibMath::Vector2 fillMax = { pos[0] + size[0] * fill, max[1] };
    if (fill > 0.f)
        context.m_drawList->DrawRectFilled(pos, fillMax, UIColorPack(m_colorFill), m_rounding);

    if (context.m_isEditor && context.m_selected)
        context.m_drawList->DrawRect(pos, max, 0xFF44AAFF, m_rounding, 1.5f);
}

void UIPanel::Draw(const UIDrawContext& context) const
{
    if (!m_visible || !context.m_drawList) return;
    LibMath::Vector2 pos = GetScreenPos(context);
    LibMath::Vector2 size = GetScreenSize(context);
    LibMath::Vector2 max = { pos[0] + size[0], pos[1] + size[1] };

    context.m_drawList->DrawRectFilled(pos, max, UIColorPack(m_colorBg), m_rounding);

    // Children use the panel's screen rect as their coordinate space.
    // origin = panel top-left, screenSize = panel pixel size.
    // PushClipRect ensures children can't draw outside the panel.
    UIDrawContext childContext = context;
    childContext.m_origin = pos;
    childContext.m_screenSize = size;

    context.m_drawList->PushClipRect(pos, max);
    for (const auto& child : m_children)
        if (child) child->Draw(childContext);
    context.m_drawList->PopClipRect();

    if (context.m_isEditor && context.m_selected)
        context.m_drawList->DrawRect(pos, max, 0xFF44AAFF, m_rounding, 1.5f);
}

void UIPanel::AddChild(std::unique_ptr<UIElement> child)
{
    m_children.push_back(std::move(child));
}

std::unique_ptr<UIElement> UIPanel::ExtractChild(size_t index)
{
    if (index >= m_children.size()) return nullptr;
    std::unique_ptr<UIElement> ptr = std::move(m_children[index]);
    m_children.erase(m_children.begin() + static_cast<ptrdiff_t>(index));
    return ptr;
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

UIElement* UICanvas::GetElement(size_t index) const
{
    return (index < m_elements.size()) ? m_elements[index].get() : nullptr;
}

std::unique_ptr<UIElement> UICanvas::ExtractElement(size_t index)
{
    if (index >= m_elements.size()) return nullptr;
    std::unique_ptr<UIElement> element = std::move(m_elements[index]);
    m_elements.erase(m_elements.begin() + static_cast<ptrdiff_t>(index));
    return element;
}

void UICanvas::Draw(const UIDrawContext& context) const
{
    for (const auto& e : m_elements)
        if (e) e->Draw(context);
}

void UICanvas::PrepareForRender(Apex::Rendering::IRHI* rhi)
{
    if (!rhi) return;
    for (auto& e : m_elements)
    {
        if (!e) continue;
        auto* img = dynamic_cast<UIImage*>(e.get());
        if (img)
        {
            if (img->m_glTexID == 0 && img->m_texture.IsReady())
            {
                Apex::Rendering::RHITextureHandle handle = img->m_texture->GetId();
                if (handle != 0)
                    img->m_glTexID = rhi->GetTexture(handle);
            }
        }
        // Recurse into panels
        auto* panel = dynamic_cast<UIPanel*>(e.get());
        if (panel)
        {
            for (auto& child : panel->m_children)
            {
                auto* childImg = dynamic_cast<UIImage*>(child.get());
                if (childImg)
                {
                    if (childImg->m_glTexID == 0 && childImg->m_texture.IsReady())
                    {
                        Apex::Rendering::RHITextureHandle h = childImg->m_texture->GetId();
                        if (h != 0)
                            childImg->m_glTexID = rhi->GetTexture(h);
                    }
                }
            }
        }
    }
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

    color.m_r = parser.ParseFloat(); 
    parser.Expect(',');

    color.m_g = parser.ParseFloat(); 
    parser.Expect(',');

    color.m_b = parser.ParseFloat(); 
    parser.Expect(',');

    color.m_a = parser.ParseFloat();
    parser.Expect(']');

    return color;
}

static LibMath::Vector2 ParseVec2(SerialParser& parser)
{
    parser.Expect('[');
    float x = parser.ParseFloat(); 
    parser.Expect(',');
    float y = parser.ParseFloat();
    parser.Expect(']');
    return { x, y };
}

static void WriteElement(std::ofstream& file, const UIElement& element, const std::string& indent)
{
    file << indent << "{\n";
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
    WriteVec2(file, "position", element.m_position, (indent + "  ").c_str()); 
    file << ",\n";
    WriteVec2(file, "size", element.m_size, (indent + "  ").c_str()); 
    file << ",\n";
    WriteColor(file, "color", element.m_color, (indent + "  ").c_str()); 
    file << ",\n";

    switch (element.GetType())
    {
        case UIWidgetType::Text:
        {
            const auto& text = static_cast<const UIText&>(element);
            file << indent << "  \"text\": \"" << text.m_text << "\",\n";
            file << indent << "  \"fontSize\": " << text.m_fontSize << "\n";
            file << indent << "  \"hAlign\": " << static_cast<int>(text.m_hAlign) << ",\n";
            file << indent << "  \"vAlign\": " << static_cast<int>(text.m_vAlign) << ",\n";
            WriteColor(file, "textColor", text.m_textColor, (indent + "  ").c_str()); 
            file << "\n";
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
            file << indent << "  \"label\": \"" << button.m_labelText.m_text << "\",\n";
            file << indent << "  \"fontSize\": " << button.m_labelText.m_fontSize << ",\n";
            WriteColor(file, "colorHover", button.m_colorHover, (indent + "  ").c_str()); 
            file << ",\n";
            WriteColor(file, "colorPressed", button.m_colorPressed, (indent + "  ").c_str()); 
            file << ",\n";
            WriteColor(file, "colorText", button.m_labelText.m_textColor, (indent + "  ").c_str()); 
            file << "\n";
            break;
        }
        case UIWidgetType::ProgressBar:
        {
            const auto& progressBar = static_cast<const UIProgressBar&>(element);
            file << indent << "  \"value\": " << progressBar.m_value << ",\n";
            file << indent << "  \"rounding\": " << progressBar.m_rounding << ",\n";
            WriteColor(file, "colorBg", progressBar.m_colorBg, (indent + "  ").c_str()); 
            file << ",\n";
            WriteColor(file, "colorFill", progressBar.m_colorFill, (indent + "  ").c_str()); 
            file << "\n";
            break;
        }
        case UIWidgetType::Panel:
        {
            const auto& panel = static_cast<const UIPanel&>(element);
            file << indent << "  \"rounding\": " << panel.m_rounding << ",\n";
            WriteColor(file, "colorBg", panel.m_colorBg, (indent + "  ").c_str()); 
            file << ",\n";
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

        auto* text = dynamic_cast<UIText*>(element.get());
        auto* image = dynamic_cast<UIImage*>(element.get());
        auto* button = dynamic_cast<UIButton*>(element.get());;
        auto* progressBar = dynamic_cast<UIProgressBar*>(element.get());
        auto* panel = dynamic_cast<UIPanel*>(element.get());

        // Base fields
        if (key == "name")          element->m_name = parser.ParseString();
        else if (key == "anchor")   element->m_anchor = AnchorFromName(parser.ParseString());
        else if (key == "visible")  element->m_visible = parser.ParseBool();
        else if (key == "position") element->m_position = ParseVec2(parser);
        else if (key == "size")     element->m_size = ParseVec2(parser);
        else if (key == "color")    element->m_color = ParseColor(parser);

        // Type-specific fields
        else if (text)
        {
            if (key == "text")     text->m_text = parser.ParseString();
            else if (key == "fontSize") text->m_fontSize = parser.ParseFloat();
            else if (key == "hAlign")    text->m_hAlign = static_cast<TextHAlign>(parser.ParseInt());
            else if (key == "vAlign")    text->m_vAlign = static_cast<TextVAlign>(parser.ParseInt());
            else if (key == "textColor") text->m_textColor = ParseColor(parser);
        }
        else if (image)
        {
            if (key == "texture")
            {
                image->m_texturePath = parser.ParseString();
                if (resourceManager && !image->m_texturePath.empty())
                    image->LoadTexture(*resourceManager);
            }
        }
        else if (button)
        {
            if (key == "label")             button->m_labelText.m_text = parser.ParseString();
            else if (key == "fontSize")     button->m_labelText.m_fontSize = parser.ParseFloat();
            else if (key == "colorHover")   button->m_colorHover = ParseColor(parser);
            else if (key == "colorPressed") button->m_colorPressed = ParseColor(parser);
            else if (key == "colorText")    button->m_labelText.m_textColor = ParseColor(parser);
        }
        else if (progressBar)
        {
            if (key == "value")          progressBar->m_value = parser.ParseFloat();
            else if (key == "rounding")  progressBar->m_rounding = parser.ParseFloat();
            else if (key == "colorBg")   progressBar->m_colorBg = ParseColor(parser);
            else if (key == "colorFill") progressBar->m_colorFill = ParseColor(parser);
        }
        else if (panel)
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


bool UICanvas::Save(const Fs::path& path) const
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

bool UICanvas::Load(const Fs::path& path, ResourceManager& rm)
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

UIManager::UIManager(IGUI* gui, ResourceManager* resourceManager, Apex::Rendering::IRHI* rhi)
    : m_gui(gui), m_resourceManager(resourceManager), m_rhi(rhi)
{
}

void UIManager::ScanHUDs(const Fs::path& assetsRoot)
{
    m_registry.clear();
    for (const auto& element : Fs::recursive_directory_iterator(assetsRoot))
    {
        if (element.is_regular_file() && element.path().extension() == ".hud")
        {
            std::string name = element.path().stem().string();
            m_registry[name] = element.path();

            LOG_INFO_CAT("HUD", "Registered HUD: {} -> {}", name, element.path().string());
        }
    }
}

int UIManager::CreateHUD(const std::string& name)
{
    // Return existing handle if already created
    for (int i = 0; i < static_cast<int>(m_instances.size()); ++i)
        if (m_instances[i] && m_instances[i]->m_name == name)
            return i + 1;

    // Look up in registry
    auto it = m_registry.find(name);
    if (it == m_registry.end())
    {
        // Try loading directly as a path relative to Assets/
        Fs::path direct = Fs::path("Assets") / (name + ".hud");
        if (!Fs::exists(direct))
        {
            LOG_ERROR_CAT("HUD", "CreateHUD: '{}' not found", name);
            return 0;
        }
        m_registry[name] = direct;
        it = m_registry.find(name);
    }

    auto inst = std::make_unique<HUDInstance>();
    inst->m_name = name;
    inst->m_visible = false;

    if (!m_resourceManager || !inst->m_canvas.Load(it->second, *m_resourceManager))
    {
        LOG_ERROR_CAT("HUD", "CreateHUD: failed to load '{}'", it->second.string());
        return 0;
    }

    m_instances.push_back(std::move(inst));
    int handle = static_cast<int>(m_instances.size());
    return handle;
}

void UIManager::DestroyHUD(int handle)
{
    HUDInstance* inst = GetInstance(handle);
    if (!inst) return;
    m_instances[static_cast<size_t>(handle) - 1].reset();
}

void UIManager::ShowHUD(int handle)
{
    HUDInstance* inst = GetInstance(handle);
    if (inst) inst->m_visible = true;
}

void UIManager::HideHUD(int handle)
{
    HUDInstance* inst = GetInstance(handle);
    if (inst) inst->m_visible = false;
}

bool UIManager::IsHUDVisible(int handle) const
{
    const HUDInstance* inst = GetInstance(handle);
    return inst ? inst->m_visible : false;
}

void UIManager::ClearAll()
{
    m_instances.clear();
}

HUDInstance* UIManager::GetInstance(int handle)
{
    if (handle < 1 || handle > static_cast<int>(m_instances.size())) return nullptr;
    return m_instances[static_cast<size_t>(handle) - 1].get();
}

const HUDInstance* UIManager::GetInstance(int handle) const
{
    if (handle < 1 || handle > static_cast<int>(m_instances.size())) return nullptr;
    return m_instances[static_cast<size_t>(handle) - 1].get();  
}

void UIManager::RenderHUD(IDrawList* drawList, LibMath::Vector2 viewportPos, LibMath::Vector2 viewportSize)
{
    if (!m_gui || !drawList) return;

    LibMath::Vector2 screen = (viewportSize[0] > 0.f && viewportSize[1] > 0.f) ? viewportSize : m_gui->GetScreenSize();
    UIDrawContext context;
    context.m_drawList = drawList;
    context.m_rhi = m_rhi;
    context.m_screenSize = screen;
    context.m_isEditor = false;
    context.m_selected = false;
    context.m_origin = viewportPos;

    for (auto& inst : m_instances)
    {
        if (!inst || !inst->m_visible) continue;
        inst->m_canvas.PrepareForRender(m_rhi);
        inst->m_canvas.Draw(context);
    }
}
