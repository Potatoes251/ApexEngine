#include "Animation.h"

#include "AnimationImporter.h"

void Animation::LoadFromFile()
{
	AnimationImporter importer;
	importer.Import(m_name, *this);
}
