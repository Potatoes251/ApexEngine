#include "OpenGLRHI.h"

#include "Log.h"

#include <fstream>
#include <sstream>
#include <cassert>

namespace Apex::Rendering
{
	IRHI* CreateOpenGLRHI() { return new OpenGLRHI(); }

	// ── Constructor ─────────────────────────────────────────────
	OpenGLRHI::OpenGLRHI()
	{
		Initialize();
	}

	// ── Init / Clear ─────────────────────────────────────────────
	void OpenGLRHI::Initialize()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	void OpenGLRHI::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);

	}
	void OpenGLRHI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRHI::ClearDepth()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRHI::SetCullingFace(bool front)
	{
		glCullFace(front ? GL_FRONT : GL_BACK);
	}

	void OpenGLRHI::SetCullingEnabled(bool enabled)
	{
		if (enabled)	glEnable(GL_CULL_FACE);
		else			glDisable(GL_CULL_FACE);
	}

	// ── VAO ───────────────────────────────────────────────────────
	RHIVAOHandle OpenGLRHI::CreateVAO() 
	{
		GLuint glVAO;
		glGenVertexArrays(1, &glVAO);
		RHIVAOHandle vao = NewHandle();
		m_vaos[vao] = glVAO;
		return vao;
	}
	void OpenGLRHI::BindVAO(RHIVAOHandle vao) 
	{
		glBindVertexArray(GetVAO(vao));
	}
	void OpenGLRHI::DeleteVAO(RHIVAOHandle vao) 
	{
		RHIVAOHandle vaoToDelete = GetVAO(vao);
		glDeleteVertexArrays(1, &vaoToDelete);
		m_vaos.erase(vao);
	}

	// ── Buffers ─────────────────────────────────────────────────
	static GLenum GetGLBufferType(BufferType type)
	{
		switch (type) 
		{
		case BufferType::Vertex: return GL_ARRAY_BUFFER;
		case BufferType::Index:  return GL_ELEMENT_ARRAY_BUFFER;
		default:                 return GL_ARRAY_BUFFER; // Default to vertex buffer
		}
	}

	RHIBufferHandle OpenGLRHI::CreateBuffer(BufferType type, const void* data, size_t sizeBytes, bool isStatic) 
	{
		GLuint glBuffer;
		glGenBuffers(1, &glBuffer);
		glBindBuffer(GetGLBufferType(type), glBuffer);
		glBufferData(GetGLBufferType(type), sizeBytes, data, isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
		RHIBufferHandle buffer = NewHandle();
		m_buffers[buffer] = glBuffer;
		return buffer;
	}

	RHIBufferHandle OpenGLRHI::CreateSSBO(Vertex* data, size_t sizeBytes)
	{
		GLuint glBuffer;
		glGenBuffers(1, &glBuffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, glBuffer);

		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeBytes, data, GL_DYNAMIC_COPY);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		RHIBufferHandle buffer = NewHandle();
		m_buffers[buffer] = glBuffer;
		return buffer;
	}

	void OpenGLRHI::UpdateBuffer(RHIBufferHandle buffer, const void* data, size_t sizeBytes) 
	{
		GLuint glBuffer = GetBuffer(buffer);
		glBindBuffer(GL_ARRAY_BUFFER, glBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeBytes, data);
	}

	void OpenGLRHI::BindBuffer(RHIBufferHandle buffer, BufferType type) 
	{
		glBindBuffer(GetGLBufferType(type), GetBuffer(buffer));
	}

	void OpenGLRHI::BindStorageBuffer(uint32_t binding, RHIBufferHandle buffer)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, GetBuffer(buffer));
	}

	void OpenGLRHI::DeleteBuffer(RHIBufferHandle buffer) 
	{
		GLuint glBuffer = GetBuffer(buffer);
		glDeleteBuffers(1, &glBuffer);
		m_buffers.erase(buffer);
	}

	RHIFrameBufferHandle OpenGLRHI::CreateFrameBuffer(unsigned width, unsigned height, RHITextureHandle& texture_out)
	{
		GLuint glFbo;
		GLuint colorTexture;
		GLuint depthTexture;

		glGenFramebuffers(1, &glFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, glFbo);

		// Color texture
		glGenTextures(1, &colorTexture);
		glBindTexture(GL_TEXTURE_2D, colorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

		// Depth
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

		RHITextureHandle texture = NewHandle();
		m_textures[texture] = colorTexture;
		texture_out = texture;

		RHITextureHandle depthHandle = NewHandle();
		m_textures[depthHandle] = depthTexture;

		RHIFrameBufferHandle fbo = NewHandle();
		m_frameBuffers[fbo] = glFbo;
		return fbo;
	}

	RHIFrameBufferHandle OpenGLRHI::CreateEmptyFrameBuffer(RHITextureHandle texture, unsigned width, unsigned height)
	{
		GLuint glFbo;

		glGenFramebuffers(1, &glFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, glFbo);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GetTexture(texture), 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		RHIFrameBufferHandle fbo = NewHandle();
		m_frameBuffers[fbo] = glFbo;
		return fbo;
	}

	void OpenGLRHI::BindFrameBuffer(RHIFrameBufferHandle buffer, unsigned viewportWidth, unsigned viewportHeight)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, GetFrameBuffer(buffer));
		glViewport(0, 0, viewportWidth, viewportHeight);
	}

	void OpenGLRHI::UnBindFrameBuffer(unsigned width, unsigned height)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
	}

	void OpenGLRHI::DeleteFrameBuffer(RHIFrameBufferHandle buffer)
	{
		GLuint glFrameBuffer = GetFrameBuffer(buffer);
		glDeleteFramebuffers(1, &glFrameBuffer);
		m_frameBuffers.erase(buffer);
	}

	int OpenGLRHI::GetPixelData(int x, int y)
	{
		GLubyte pixel[4];
		glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

		return (int(pixel[0]) << 16) | (int(pixel[1]) << 8) | int(pixel[2]);
	}

	RHIUniformBufferHandle OpenGLRHI::CreateUniformBuffer(unsigned int bindingPoint, size_t sizeBytes)
	{
		GLuint ubo;

		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeBytes, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		RHIUniformBufferHandle buffer = NewHandle();
		m_uniformBuffers[buffer] = ubo;
		return buffer;
	}

	void OpenGLRHI::BindUniformBuffer(RHIUniformBufferHandle buffer, size_t sizeBytes, void* data)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, GetuniformeBuffer(buffer));
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeBytes, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLRHI::DeleteUniformBuffer(RHIUniformBufferHandle buffer)
	{
		GLuint glFrameBuffer = GetuniformeBuffer(buffer);
		glDeleteBuffers(1, &glFrameBuffer);
		m_uniformBuffers.erase(buffer);
	}

	// ── Textures ─────────────────────────────────────────────────
	RHITextureHandle OpenGLRHI::CreateTexture(const TextureDescription& desc)
	{
		GLuint glTexture;
		glGenTextures(1, &glTexture);
		glBindTexture(GL_TEXTURE_2D, glTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		const GLenum internalFmt = (desc.m_channels == 4) ? GL_RGBA8 : GL_RGB8;
		const GLenum pixelFmt = (desc.m_channels == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, desc.m_width, desc.m_height, 0, pixelFmt, GL_UNSIGNED_BYTE, desc.m_data);
		glGenerateMipmap(GL_TEXTURE_2D);

		RHITextureHandle texture = NewHandle();
		m_textures[texture] = glTexture;
		return texture;
	}

	RHITextureHandle OpenGLRHI::CreateCubemap(const std::vector<TextureDescription>& desc)
	{
		GLuint glTexture;
		glGenTextures(1, &glTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, glTexture);

		const GLenum internalFmt = (desc[0].m_channels == 4) ? GL_RGBA8 : GL_RGB8;
		const GLenum pixelFmt = (desc[0].m_channels == 4) ? GL_RGBA : GL_RGB;

		for (int i = 0; i < 6; i++)
		{
			assert(desc[i].m_width == desc[0].m_width);
			assert(desc[i].m_height == desc[0].m_height);
			assert(desc[i].m_channels == desc[0].m_channels);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFmt, 
				desc[i].m_width, desc[i].m_height, 0, pixelFmt, GL_UNSIGNED_BYTE, desc[i].m_data);
		}

		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		RHITextureHandle texture = NewHandle();
		m_textures[texture] = glTexture;
		return texture;
	}

	RHITextureHandle OpenGLRHI::CreateShadowMap(unsigned width, unsigned height)
	{
		GLuint glTexture;
		glGenTextures(1, &glTexture);
		glBindTexture(GL_TEXTURE_2D, glTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glBindTexture(GL_TEXTURE_2D, 0);

		RHITextureHandle texture = NewHandle();
		m_textures[texture] = glTexture;
		return texture;
	}

	RHITextureHandle OpenGLRHI::CreateShadowCubeMap(unsigned width, unsigned height)
	{
		GLuint glTexture;
		glGenTextures(1, &glTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, glTexture);

		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
				width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		RHITextureHandle texture = NewHandle();
		m_textures[texture] = glTexture;
		return texture;
	}
	
	void OpenGLRHI::AttachDepthTexture(RHITextureHandle depthTexture)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GetTexture(depthTexture), 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	void OpenGLRHI::AttachDepthCubeTexture(RHITextureHandle depthTexture)
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GetTexture(depthTexture), 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	void OpenGLRHI::BindTexture(RHITextureHandle texture, uint32_t unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, GetTexture(texture));
	}

	void OpenGLRHI::BindTextureCube(RHITextureHandle texture, uint32_t unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, GetTexture(texture));
	}

	void OpenGLRHI::DeleteTexture(RHITextureHandle texture)
	{
		auto it = m_textures.find(texture);
		if (it != m_textures.end())
		{
			glDeleteTextures(1, &it->second); 
			m_textures.erase(it);             
		}
	}

	// ── Shaders ──────────────────────────────────────────────────
	RHIShaderHandle OpenGLRHI::CreateShader(const ShaderSource& source)
	{
		GLuint vertexShader = CompileShader(source.m_vertexPath, GL_VERTEX_SHADER);
		GLuint fragmentShader = CompileShader(source.m_fragmentPath, GL_FRAGMENT_SHADER);
		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetProgramInfoLog(program, 512, nullptr, infoLog);
			LOG_ERROR("Shader program linking failed : {}", infoLog);
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		RHIShaderHandle shader = NewHandle();
		m_shaders[shader] = program;
		return shader;
	}

	RHIShaderHandle OpenGLRHI::CreateShaderGeo(const ShaderSource& source)
	{
		GLuint vertexShader = CompileShader(source.m_vertexPath, GL_VERTEX_SHADER);
		GLuint fragmentShader = CompileShader(source.m_fragmentPath, GL_FRAGMENT_SHADER);
		GLuint geometryShader = CompileShader(source.m_geometryPath, GL_GEOMETRY_SHADER);
		GLuint program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glAttachShader(program, geometryShader);
		glLinkProgram(program);

		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetProgramInfoLog(program, 512, nullptr, infoLog);
			LOG_ERROR("Shader program linking failed : {}", infoLog);
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteShader(geometryShader);

		GLuint blockIndex = glGetUniformBlockIndex(program, "PointLightBuffer");
		if (blockIndex != GL_INVALID_INDEX)
		{
			glUniformBlockBinding(program, blockIndex, 0);
		}
		blockIndex = glGetUniformBlockIndex(program, "DirLightBuffer");
		if (blockIndex != GL_INVALID_INDEX)
		{
			glUniformBlockBinding(program, blockIndex, 1);
		}
		blockIndex = glGetUniformBlockIndex(program, "SpotLightBuffer");
		if (blockIndex != GL_INVALID_INDEX)
		{
			glUniformBlockBinding(program, blockIndex, 2);
		}

		RHIShaderHandle shader = NewHandle();
		m_shaders[shader] = program;
		return shader;
	}

	RHIShaderHandle OpenGLRHI::CreateComputeShader(const std::string& source)
	{
		std::string file = ReadFile(source);
		char const* content = file.c_str();

		GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &content, NULL);
		glCompileShader(compute);

		GLuint program = glCreateProgram();
		glAttachShader(program, compute);
		glLinkProgram(program);

		GLint success;
		glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetShaderInfoLog(compute, 512, nullptr, infoLog);
			LOG_ERROR("Shader program compilation failed : {}", infoLog);
		}

		success = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetProgramInfoLog(program, 512, nullptr, infoLog);
			LOG_ERROR("Shader program linking failed : {}", infoLog);
		}

		RHIShaderHandle shader = NewHandle();
		m_shaders[shader] = program;
		return shader;
	}

	void OpenGLRHI::BindShader(RHIShaderHandle shader)
	{
		glUseProgram(GetShader(shader));
	}


	void OpenGLRHI::DeleteShader(RHIShaderHandle shader)
	{
		glDeleteProgram(GetShader(shader));
		m_shaders.erase(shader);
	}

	void OpenGLRHI::Barrier()
	{
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void OpenGLRHI::DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ)
	{
		glDispatchCompute(groupX, groupY, groupZ);
	}

	// ── Uniforms ─────────────────────────────────────────────────
	void OpenGLRHI::SetUniformInt(RHIShaderHandle shader, const std::string_view& name, int value)
	{
		GLint location = glGetUniformLocation(GetShader(shader), name.data());
		glUniform1i(location, value);
	}

	void OpenGLRHI::SetUniformFloat(RHIShaderHandle shader, const std::string_view& name, float value)
	{
		GLint location = glGetUniformLocation(GetShader(shader), name.data());
		glUniform1f(location, value);
	}

	void OpenGLRHI::SetUniformFloatArray(RHIShaderHandle shader, const std::string_view& name, const float& value, int count)
	{
		GLint location = glGetUniformLocation(GetShader(shader), name.data());
		glUniform1fv(location, count, &value);
	}

	void OpenGLRHI::SetUniformVec3(RHIShaderHandle shader, const std::string_view& name, const LibMath::Vector3& vec)
	{
		GLint location = glGetUniformLocation(GetShader(shader), name.data());
		glUniform3f(location, vec[0], vec[1], vec[2]);
	}

	void OpenGLRHI::SetUniformVec4(RHIShaderHandle shader, const std::string_view& name, const LibMath::Vector4& vec)
	{
		GLint location = glGetUniformLocation(GetShader(shader), name.data());
		glUniform4f(location, vec[0], vec[1], vec[2], vec[3]);
	}

	void OpenGLRHI::SetUniformMat4(RHIShaderHandle shader, const std::string_view& name, const LibMath::Matrix4& matrix)
	{
		GLint location = glGetUniformLocation(GetShader(shader), name.data());
		glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());
	}

	void OpenGLRHI::SetUniformMat4Array(RHIShaderHandle shader, const std::string_view& name, const LibMath::Matrix4& matrix, int count)
	{
		GLint location = glGetUniformLocation(GetShader(shader), name.data());
		glUniformMatrix4fv(location, count, GL_FALSE, matrix.data());
	}

	void OpenGLRHI::SetupVertexLayout()
	{
		//mesh datas
		glEnableVertexAttribArray(0); // Position
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_position));
		glEnableVertexAttribArray(1); // Normal
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));

		//animation datas
		glEnableVertexAttribArray(2); //bone IDs
		glVertexAttribIPointer(3, 4, GL_INT,  sizeof(Vertex), (void*)offsetof(Vertex, m_boneIDs));
		glEnableVertexAttribArray(3); //bone weights
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_weights));
	}

	void OpenGLRHI::SetupVector3Layout()
	{
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LibMath::Vector3), (void*)0);
	}

	// ── Draw ───────────────────────────────────────────────────────
	void OpenGLRHI::DrawEmpty()
	{
		static GLuint vao = 0;

		glDisable(GL_CULL_FACE);

		if (vao == 0)
			glGenVertexArrays(1, &vao);

		glBindVertexArray(vao);
		DrawArrays(3);

		glEnable(GL_CULL_FACE);
	}

	void OpenGLRHI::DrawIndexed(uint32_t indexCount)
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRHI::DrawArrays(uint32_t vertexCount)
	{
		glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	}

	void OpenGLRHI::DrawLineArrays(uint32_t vertexCount)
	{
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRHI::SetDepthTest(bool enabled) 
	{
		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRHI::SetDepthTestWrite(bool enabled)
	{
		glDepthMask(enabled ? GL_TRUE : GL_FALSE);
	}


	// ── Helpers ───────────────────────────────────────────────────
	uint32_t OpenGLRHI::GetBuffer(RHIBufferHandle handle) const 
	{
		auto it = m_buffers.find(handle);
		return (it != m_buffers.end()) ? it->second : 0;
	}
	uint32_t OpenGLRHI::GetFrameBuffer(RHIFrameBufferHandle handle) const 
	{
		auto it = m_frameBuffers.find(handle);
		return (it != m_frameBuffers.end()) ? it->second : 0;
	}
	uint32_t OpenGLRHI::GetuniformeBuffer(RHIUniformBufferHandle handle) const
	{
		auto it = m_uniformBuffers.find(handle);
		return (it != m_uniformBuffers.end()) ? it->second : 0;
	}
	uint32_t OpenGLRHI::GetShader(RHIShaderHandle handle) const 
	{
		auto it = m_shaders.find(handle);
		return (it != m_shaders.end()) ? it->second : 0;
	}
	uint32_t OpenGLRHI::GetTexture(RHITextureHandle handle) const 
	{
		auto it = m_textures.find(handle);
		return (it != m_textures.end()) ? it->second : 0;
	}
	uint32_t OpenGLRHI::GetVAO(RHIVAOHandle handle) const 
	{
		auto it = m_vaos.find(handle);
		return (it != m_vaos.end()) ? it->second : 0;
	}

	GLuint OpenGLRHI::CompileShader(const std::string& path, GLenum type)
	{
		std::string source = ReadFile(path);
		const char* sourceCStr = source.c_str();
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &sourceCStr, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			LOG_ERROR("Shader program compilation failed : {}", infoLog);
		}
		return shader;
	}

	std::string OpenGLRHI::ReadFile(const std::string& path)
	{
		std::ifstream file(path);
		if (!file.is_open())
		{
			LOG_ERROR("couldnt open file : {}", path);
			return "";
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
}