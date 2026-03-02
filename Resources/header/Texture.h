#ifndef TEXTURE
#define TEXTURE

#include "IResource.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

class Texture : public IResource
{
public:
	Texture(std::string name) : m_path(name) {}
	Texture(Texture const&) = delete;
	Texture& operator=(Texture const&) = delete;

	~Texture() { if (m_id) glDeleteTextures(1, &m_id); }

	GLuint GetId() const { return m_id; }

	void LoadFromFile() override;
	void UploadToGpu() override;

	void Bind(GLuint slot = 0);

private:
	std::string m_path;
	GLuint		m_id		= 0;
	int			m_channels	= 0;
	int			m_width		= 0;
	int			m_height	= 0;
	std::vector<unsigned char> m_imageData;
};


#endif // !TEXTURE

