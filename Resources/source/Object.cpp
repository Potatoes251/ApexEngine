#include "Object.h"

using namespace Apex::Data;

Object::Object(const Object& other) :
	m_sceneNode(SceneNode(other.m_sceneNode.GetLocalTransform())) {}

Matrix4 Object::GetTransform()
{
	return m_sceneNode.GetLocalTransform();
}
