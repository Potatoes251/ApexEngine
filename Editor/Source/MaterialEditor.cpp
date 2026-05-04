#include "MaterialEditor.h"

#include "Shader.h"

#include <sstream>

using namespace Apex::Editor;
using namespace Apex::Rendering;

void MaterialEditor::Open(std::string path)
{
    m_open = true;
    m_currentMat = m_resourceManager->CreateAsync<Material>(path);
}

void MaterialEditor::Close()
{
    m_open = false;
    m_vertShadPickOpen = false;
    m_fragShadPickOpen = false;
    m_currentMat = nullptr;
}

void MaterialEditor::Draw()
{
	if (!m_open) return;

	m_gui->BeginPanel("MaterialEditor", &m_open, false, true);

    if (!m_currentMat.IsReady())
    {
        m_gui->Text("Loading...");
        m_gui->EndPanel();
        return;
    }
    // first frame ready
    if (!m_wasReady && m_currentMat->GetShader().IsReady())
    {
        std::string const& shaders = m_currentMat->GetShader()->GetPath();

        if (!shaders.empty())
        {
            std::stringstream ss(shaders);

            std::getline(ss, m_vertexShader, '|');
            std::getline(ss, m_fragmentShader, '|');
        }

        if (m_currentMat->GetComputeShader().IsValid())
        {
            m_computeShader = m_currentMat->GetComputeShader()->GetPath();
        }
    }

    m_wasReady = true;

    if (!m_open)
    {
        Close();
        m_gui->EndPanel();
        return;
    }

    if (m_gui->Button("Add Texture"))
        m_currentMat->AddEmptyTexture();

    m_gui->SameLine();
    if (m_gui->Button("Save"))
    {
        std::string shaders = m_vertexShader + '|' + m_fragmentShader;
        m_currentMat->SetShader(m_resourceManager->CreateAsync<Shader>(shaders));
        m_currentMat->SetComputeShader(m_resourceManager->CreateAsync<ComputeShader>(m_computeShader));

        while (!m_currentMat->GetShader().IsReady() && !m_currentMat->GetShader().IsFailed())
            m_resourceManager->ProcessGpuUploads();

        m_currentMat->Save();
    }

    std::vector<Resources::ResourceHandle<Texture>>& textures = m_currentMat->GetTextures();
    for (int i = 0; i < textures.size(); i++)
    {
        DrawTexture(&textures[i], i);
    }

    DrawShaderPicker(m_computeShader, m_compShadPickOpen, "Compute");
    DrawShaderPicker(m_vertexShader, m_vertShadPickOpen, "Vertex");
    DrawShaderPicker(m_fragmentShader, m_fragShadPickOpen, "Fragment");
    m_texturePicker.Draw();


	m_gui->EndPanel();
}

void MaterialEditor::DrawTexture(Resources::ResourceHandle<Texture>* texture, int id)
{
    m_gui->AlignTextToFramePadding();
    m_gui->Text("Texture:");
    m_gui->SameLine();
    m_gui->TextDisabled(texture->IsValid() ? (*texture)->GetPath() : "None");

    float width = m_gui->GetAvailableSize()[0];
    constexpr float height = 22.f;
    if (m_gui->ButtonSized(std::string("Select##") + (*texture)->GetPath() + std::to_string(id), {width, height }))
        m_texturePicker.Open(texture);
}

void MaterialEditor::DrawShaderPicker(std::string& shaderPath, bool& open, std::string shaderType)
{
    m_gui->AlignTextToFramePadding();
    m_gui->Text(shaderType + " Shader:");
    m_gui->SameLine();

    m_gui->TextDisabled(shaderPath.empty() ? "None" : shaderPath.c_str());

    float width = m_gui->GetAvailableSize()[0];
    constexpr float height = 22.f;
    if (m_gui->ButtonSized("Select " + shaderType + " Shader", { width, height }))
    {
        open = true;
    }

    if (!open) return;

    m_gui->BeginModalBlock("Select " + shaderType + " Shader");

    m_gui->BeginPanel("Select " + shaderType + " Shader", &open, false, true);

    if (!std::filesystem::exists("ApexAssets/") || !open)
    {
        m_gui->EndPanel();
        m_gui->EndModalBlock();
        open = false;
        return;
    }
    
    for (auto& entry : std::filesystem::recursive_directory_iterator("ApexAssets/"))
    {
        if (!entry.is_regular_file())
            continue;

        std::filesystem::path ext = entry.path().extension();
        if (ext != ".glsl" && ext != ".frag" && ext != ".vert")
            continue;

        std::string path = entry.path().generic_string();

        if (m_gui->Button(path.c_str()))
        {
           if (shaderPath != path)
           {
               shaderPath = path;
               open = false;
           }
        }
    }

    m_gui->EndPanel();

    m_gui->EndModalBlock();
}
