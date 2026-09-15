#ifndef SSBO
#define SSBO

#include "RHI.h"

namespace Apex::Rendering
{
	class Ssbo
	{
	public:
		Ssbo() = default;
		Ssbo(IRHI* rhi, Vertex* data, size_t size, uint32_t binding)
			: m_rhi(rhi), m_size(size), m_binding(binding)
		{
			m_buffer = m_rhi->CreateSSBO(data, size);
		}
		
		Ssbo(Ssbo&& other) noexcept
			: m_rhi(other.m_rhi), m_size(other.m_size), m_binding(other.m_binding), m_buffer(other.m_buffer)
		{
			other.m_rhi = nullptr;
			other.m_size = 0;
			other.m_binding = 0;
			other.m_buffer = 0;
		}

		Ssbo& operator=(Ssbo&& other) noexcept
		{
			if (this != &other)
			{
				if (m_rhi) m_rhi->DeleteBuffer(m_buffer);

				m_rhi = other.m_rhi;
				m_size = other.m_size;
				m_binding = other.m_binding;
				m_buffer = other.m_buffer;

				other.m_rhi = nullptr;
				other.m_size = 0;
				other.m_binding = 0;
				other.m_buffer = 0;
			}
			return *this;
		}

		~Ssbo() { if (m_rhi) m_rhi->DeleteBuffer(m_buffer); }

		void Bind() const { m_rhi->BindStorageBuffer(m_binding, m_buffer); }
		uint32_t GetId() const { return m_buffer; }
	private:
		IRHI* m_rhi = nullptr;

		size_t m_size = 0;
		uint32_t m_binding = 0;
		uint32_t m_buffer = 0;
	};
}


#endif // !SSBO