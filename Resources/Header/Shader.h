#ifndef SHADER
#define SHADER

#include "IResource.h"
#include "RHI.h"

#include <string>
#include <vector>

class Shader : public IResource
{
public:
	Shader(std::string name) : IResource(name) {}
	Shader(Shader const&) = delete;
	Shader& operator=(Shader const&) = delete;

	~Shader() { m_rhi->DeleteShader(m_shader); }

	void UploadToGpu() override;

	void Use() const;

	void SetUniform(std::string_view const& name, int value);
	void SetUniform(std::string_view const& name, float value);
	void SetUniform(std::string_view const& name, const float& value, int count);
	void SetUniform(std::string_view const& name, LibMath::Vector3 const& value);
	void SetUniform(std::string_view const& name, LibMath::Matrix4 const& value);
	void SetUniform(std::string_view const& name, LibMath::Matrix4 const& value, int count);

private:
	uint32_t m_shader = 0;
};


#endif // !SHADER

