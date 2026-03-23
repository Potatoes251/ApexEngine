#ifndef SHADER
#define SHADER

#include "IResource.h"

#include <string>
#include <vector>

#include <glad/glad.h>

class Shader : public IResource
{
public:
	Shader(std::string name) : m_name(name) {}
	Shader(Shader const&) = delete;
	Shader& operator=(Shader const&) = delete;

	~Shader() { m_rhi->DeleteShader(m_shader); }

	void UploadToGpu() override;

	void Use() const;

	void SetUniform(std::string const& name, int value);
	void SetUniform(std::string const& name, float value);
	void SetUniform(std::string const& name, LibMath::Vector3 const& value);
	void SetUniform(std::string const& name, LibMath::Matrix4 const& value);
	void SetUniformMat4Array(std::string const& name, std::vector<LibMath::Matrix4> const& value);

private:
	std::string m_name;

	GLuint m_shader = 0;
};


#endif // !SHADER

