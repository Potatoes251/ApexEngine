#ifndef DEBUG_RENDERER
#define DEBUG_RENDERER

#include <vector>
#include <cassert>

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Quaternion.h"
#include "LibMath/Matrix/Matrix4.h"

#include "ResourceHandle.h"
#include "Shader.h"
#include "RHI.h"

namespace Apex::Rendering
{
	struct Line
	{
		LibMath::Vector3 m_start;
		LibMath::Vector3 m_end;
	};

	class DebugRenderer
	{
	public:
		DebugRenderer(DebugRenderer const&) = delete;
		DebugRenderer& operator=(DebugRenderer const&) = delete;

		static void	Initialize(Apex::Resources::ResourceHandle<Shader> shader);

		static DebugRenderer& Get();

		void AddBox(LibMath::Vector3 pos, LibMath::Quaternion rotation, LibMath::Vector3 halfExtents);
		void AddCapsule(LibMath::Vector3 pos, LibMath::Quaternion rotation, float radius, float halfHeight);
		void AddLine(LibMath::Vector3 m_start, LibMath::Vector3 m_end);

		void DrawLines(IRHI* rhi, LibMath::Matrix4 viewProj);

	private:
		DebugRenderer(Apex::Resources::ResourceHandle<Shader> shader);

		static inline std::unique_ptr<DebugRenderer>	m_instance;

		uint32_t m_vao = 0;
		uint32_t m_vbo = 0;

		std::vector<Line> m_lines;

		Apex::Resources::ResourceHandle<Shader> m_shader;
	};
}


#endif // !DEBUG_RENDERER

