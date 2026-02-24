#include "Object.h"

using namespace Apex::Data;

Matrix4 Object::GetTransform()
{
	return m_sceneNode.GetLocalTransform();
}
