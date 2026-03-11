#include "Animation.h"

#include "AnimationImporter.h"

Animation::Animation(string name) : m_name(name) {}

void Animation::LoadFromFile()
{
	AnimationImporter importer;
	importer.Import(m_name, *this);
}
