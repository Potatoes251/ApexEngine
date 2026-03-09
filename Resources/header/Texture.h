#ifndef TEXTURE
#define TEXTURE

#include "IResource.h"

#include <string>
#include <vector>

class Texture : public IResource
{
public:
	Texture(std::string name) : m_path(name) {}
	Texture(Texture const&) = delete;
	Texture& operator=(Texture const&) = delete;

	~Texture() { m_rhi->DeleteTexture(m_id); }

	uint32_t GetId() const { return m_id; }

	void LoadFromFile() override;
	void UploadToGpu() override;

	void Bind(uint32_t slot = 0);

private:
	std::string m_path;
	uint32_t	m_id		= 0;
	int			m_channels	= 0;
	int			m_width		= 0;
	int			m_height	= 0;
	std::vector<unsigned char> m_imageData;
};


#endif // !TEXTURE

