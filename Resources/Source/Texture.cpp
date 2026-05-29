#include "Texture.h"

#include "RHI.h"

#include <stb_image.h>

#include <iostream>
#include "Cubemap.h"

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

    SetState(ResourceState::Loaded);
}

void Texture::UploadToGpu()
{
    if (m_imageData.empty())
    {
        std::cout << "No image data to upload for texture: " << m_path << std::endl;
        SetState(ResourceState::Failed);
        return;
    }
    
    m_id = m_rhi->CreateTexture(Apex::Rendering::TextureDescription(m_width, m_height, m_channels, m_imageData.data()));

    m_imageData.clear();
    m_imageData.shrink_to_fit();
    SetState(ResourceState::Uploaded);
}

void Texture::Bind(uint32_t slot)
{
    m_rhi->BindTexture(m_id, slot);
}
