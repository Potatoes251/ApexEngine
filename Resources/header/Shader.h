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

	~Shader() { if (m_shaderProgram) glDeleteProgram(m_shaderProgram); }

	void UploadToGpu() override;

	void Use() const;
	GLuint GetProgram() const { return m_shaderProgram; }

private:
	bool	CreateShader(std::vector<std::string> const& files);
	bool	InitShader(std::string const& shaderFile, GLuint& shader) const;				// load a shader
	bool	Link(GLuint vertexShader, GLuint fragmentShader, GLuint geometryShader) const;	// link into a shader program
	void	Cleanup(GLuint vertexShader, GLuint fragmentShader, GLuint geometryShader);


	std::string m_name;

	GLuint m_shaderProgram = 0;
};


#endif // !SHADER

