#include "Model.h"

#include "Mesh.h"

#include "MeshLoader.h"

void Model::LoadFromFile()
{
	MeshLoader loader;
	loader.Load(m_path, *this);

	for (Mesh& mesh : m_meshes)
	{
		mesh.SetRhi(m_rhi);
		for (const auto& v : mesh.m_vertices)
		{
			for (int c = 0; c < 3; ++c)
			{
				if (v.m_position[c] < m_boundsMin[c]) m_boundsMin[c] = v.m_position[c];
				if (v.m_position[c] > m_boundsMax[c]) m_boundsMax[c] = v.m_position[c];
			}
		}
	}

	SetState(ResourceState::Loaded);
}

void Model::UploadToGpu()
{
	for (Mesh& mesh : m_meshes)
	{
		mesh.UploadToGpu();
	}
	SetState(ResourceState::Uploaded);
}

int Model::GetAnimationIdx(std::string const& name)
{
	for (int i = 0; i < m_animations.size(); i++)
	{
		Animation const& anim = m_animations[i];
		if (anim.m_name == name)
			return i;
	}
	return -1;
}