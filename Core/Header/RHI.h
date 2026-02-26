#ifndef RHI
#define RHI

// ============================================================
// RHI.h — Render Hardware Interface
// ============================================================

#include <glad/glad.h>

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Matrix/Matrix4.h"

#include <string>
#include <vector>

namespace Apex::Rendering
{
	using RHIBufferHandle = GLuint;
	using RHIVAOHandle = GLuint;

	constexpr GLuint RHI_INVALID = 0;

	// ── Buffer types ────────────────────────────────────────────── 
	enum class BufferType
	{
		Vertex,
		Index
	};

	// ── Shader source ─────────────────────────────────────────────
	struct ShaderSource 
	{
		std::string vertexPath;
		std::string fragmentPath;
	};
	// ── RHI abstract interface ────────────────────────────────────
	class IRHI
	{
	public:
		virtual ~IRHI() = default;

		virtual void Initialize() = 0;
		virtual void Clear(float r, float g, float b, float a = 1.f) = 0;

		// VAO
		virtual RHIVAOHandle	CreateVAO() = 0;
		virtual void			BindVAO(RHIVAOHandle vao) = 0;
		virtual void			DeleteVAO(RHIVAOHandle vao) = 0;

		// Buffers
		virtual RHIBufferHandle		CreateBuffer(BufferType type, const void* data, size_t sizeBytes) = 0;
		virtual void				UpdateBuffer(RHIBufferHandle buffer, const void* data, size_t sizeBytes) = 0;
		virtual void				BindBuffer(RHIBufferHandle buffer, BufferType type) = 0;
		virtual void				DeleteBuffer(RHIBufferHandle buffer) = 0;

		// Uniforms (bound shader must be active)
		virtual void SetUniformInt(GLuint program, const std::string& name, int value) = 0;
		virtual void SetUniformFloat(GLuint program, const std::string& name, float value) = 0;
		virtual void SetUniformVec3(GLuint program, const std::string& name, const LibMath::Vector3& vec) = 0;
		virtual void SetUniformMat4(GLuint program, const std::string& name, const LibMath::Matrix4& matrix) = 0;

		// Draw
		virtual void DrawIndexed(GLsizei indexCount) = 0;
		virtual void DrawArrays(GLsizei vertexCount) = 0;
		virtual void SetDepthTest(bool enabled) = 0;
	};

	// Factory
	IRHI* CreateOpenGLRHI();

} // namespace Apex::Rendering

#endif