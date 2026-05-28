#include "Model.h"

#include "Mesh.h"

#include "MeshLoader.h"

void Model::LoadFromFile()
{
	MeshLoader loader;
	loader.Load(m_path, *this);

	for (Mesh& mesh : m_meshes)
	{
		mesh.SetBounds();
	}
}

void Model::UploadToGpu()
{
	for (Mesh& mesh : m_meshes)
	{
		mesh.UploadToGpu();
	}
}
