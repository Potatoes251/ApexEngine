#include "Viewport.h"
#include "stb_image.h"
#include <glad/glad.h>
using namespace Apex::Data;

using namespace Apex::Editor;
using namespace Apex::Rendering;

GLuint LoadTexture(const char* path)
{
    int width, height, channels;

    // Flip because OpenGL expects 0,0 bottom-left
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data)
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLenum format;
    if (channels == 1)      format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;
    else
    {
        std::cout << "Unsupported texture format\n";
        stbi_image_free(data);
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

Viewport::Viewport(Resources::ResourceHandle<Shader> shader, Rendering::Scene* scene, Rendering::IRHI* rhi, UserInterface::IGUI* gui)
	: m_gizmo(shader, rhi), m_currentScene(scene), m_rhi(rhi), m_gui(gui)
{
	m_fbo = m_rhi->CreateFrameBuffer(m_width, m_height, m_texture);
    //m_texture = LoadTexture("Assets/Textures/Potatoes.jpg");
}

void Viewport::Update()
{
	
}

void Viewport::Draw()
{
	LibMath::Matrix4 view = m_camera.GetViewMatrix();
	LibMath::Matrix4 proj = m_camera.GetProjectionMatrix(m_width / m_height);

	m_rhi->BindFrameBuffer(m_fbo, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_currentScene->Render(view, proj, m_camera.GetPosition());
    m_rhi->SetDepthTest(false);
	m_gizmo.DrawGizmos(m_selectedObject, m_currentGizmoMode, proj * view);
    m_rhi->SetDepthTest(true);
    m_rhi->UnBindFrameBuffer();

	m_gui->BeginPanel("Viewport");
	LibMath::Vector2 pos = m_gui->GetPanelPos() + (m_gui->GetPanelSize() * 0.5f);

	m_gui->DrawImage2D(m_rhi->GetTexture(m_texture), m_gui->GetPanelPos(), m_gui->GetPanelPos() + m_gui->GetPanelSize());

	m_gui->EndPanel();
}