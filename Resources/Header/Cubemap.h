#ifndef CUBEMAP
#define CUBEMAP

#include "IResource.h"
#include "RHI.h"

class Cubemap : public IResource
{
public:
	Cubemap(std::string path) : IResource(path) {}
	Cubemap(Cubemap const&) = delete;
	Cubemap& operator=(Cubemap const&) = delete;

	~Cubemap() { if (m_rhi && m_id != 0) m_rhi->DeleteTexture(m_id); }

	void Bind(uint32_t slot = 0);

	void LoadFromFile() override;
	void UploadToGpu() override;

private:
	uint32_t	m_id = 0;
	std::vector<Apex::Rendering::TextureDescription> m_descs;
};

#endif // !CUBEMAP

