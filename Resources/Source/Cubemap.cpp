#include "Cubemap.h"

#include "Log.h"

#include "stb_image.h"
#include <sstream>

void Cubemap::LoadFromFile()
{
	std::stringstream ss(m_path);
	std::string item;
	stbi_set_flip_vertically_on_load(true);

	while (std::getline(ss, item, '|')) // split by '|'
	{
		Apex::Rendering::TextureDescription desc;

		desc.m_data = stbi_load(item.c_str(), &desc.m_width, &desc.m_height, &desc.m_channels, 0);

		if (!desc.m_data)
		{
			LOG_WARNING("Failed to load texture: {}", item);
			SetState(ResourceState::Failed);
			for (Apex::Rendering::TextureDescription& description : m_descs) stbi_image_free(description.m_data);

			m_descs.clear();
			m_descs.shrink_to_fit();
			return;
		}
		m_descs.push_back(desc);
	}

	if (m_descs.size() != 6)
	{
		LOG_WARNING("Cubemap requires exactly 6 textures");
		for (auto& d : m_descs)
			stbi_image_free(d.m_data);
		m_descs.clear();
		m_descs.shrink_to_fit();
		SetState(ResourceState::Failed);
		return;
	}

	SetState(ResourceState::Loaded);
}

void Cubemap::UploadToGpu()
{
	m_id = m_rhi->CreateCubemap(m_descs);

	for (Apex::Rendering::TextureDescription& desc : m_descs) stbi_image_free(desc.m_data);

	m_descs.clear();
	m_descs.shrink_to_fit();
	SetState(ResourceState::Uploaded);
}

void Cubemap::Bind(uint32_t slot)
{
	m_rhi->BindTextureCube(m_id, slot);
}