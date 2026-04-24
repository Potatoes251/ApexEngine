#ifndef SSBO
#define SSBO

#include "RHI.h"

namespace Apex::Rendering
{
	class Ssbo
	{
	public:
		Ssbo() = default;
		Ssbo(IRHI* rhi, size_t size, uint32_t binding)
			: m_rhi(rhi), m_size(size), m_binding(binding)
		{
			m_buffer = m_rhi->CreateSSBO(size);
		}

		~Ssbo() { if (m_rhi) m_rhi->DeleteBuffer(m_buffer); }

		void Bind() { m_rhi->BindStorageBuffer(m_binding, m_buffer); }

		void Barrier() { m_rhi->Barrier(); }
	private:
		IRHI* m_rhi = nullptr;

		size_t m_size = 0;
		uint32_t m_binding = 0;
		uint32_t m_buffer = 0;
	};
}


#endif // !SSBO