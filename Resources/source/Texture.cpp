#include "Texture.h"

#include <stb_image.h>

#include <iostream>

void Texture::LoadFromFile()
{
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(m_path.c_str(), &m_width, &m_height, &m_channels, 0);

	if (!data)
	{
		std::cout << "Failed to load texture: " << m_path << std::endl;
        SetState(ResourceState::Failed);
		return;
	}

	m_imageData.assign(data, data + (m_width * m_height * m_channels));
	stbi_image_free(data);
}

void Texture::UploadToGpu()
{
    if (m_imageData.empty())
    {
        std::cout << "No image data to upload for texture: " << m_path << std::endl;
        SetState(ResourceState::Failed);
        return;
    }

    GLenum internalFmt = GL_RGB8;
    GLenum pixelFmt = GL_RGB;

    if (m_channels == 1) { internalFmt = GL_R8;    pixelFmt = GL_RED; }
    else if (m_channels == 3) { internalFmt = GL_RGB8;  pixelFmt = GL_RGB; }
    else if (m_channels == 4) { internalFmt = GL_RGBA8; pixelFmt = GL_RGBA; }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, m_width, m_height, 0, pixelFmt, GL_UNSIGNED_BYTE, m_imageData.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_imageData.clear();
    m_imageData.shrink_to_fit();
}

void Texture::Bind(GLuint slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_id);
}
