#ifndef COMPUTE_SHADER
#define COMPUTE_SHADER


#include "IResource.h"
#include "RHI.h"

class ComputeShader : public IResource
{
public:
	ComputeShader(std::string path) : IResource(path) {}
	~ComputeShader() { m_rhi->DeleteShader(m_shader); }

	void UploadToGpu() override;

	void Use();

	void Dispatch(uint32_t groupX, uint32_t groupY, uint32_t groupZ);

	void SetUniform(std::string_view const& name, int value);
	void SetUniform(std::string_view const& name, float value);
	void SetUniform(std::string_view const& name, LibMath::Vector3 const& value);
	void SetUniform(std::string_view const& name, LibMath::Matrix4 const& value);
	void SetUniform(std::string_view const& name, LibMath::Matrix4 const& value, int count);

private:
	uint32_t m_shader = 0;
};

#endif // !COMPUTE_SHADER

