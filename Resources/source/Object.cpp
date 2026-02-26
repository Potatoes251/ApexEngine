#include "Object.h"
#include "../../LowRenderer/header/SceneGraph.h"

using namespace Apex::Data;

Apex::Data::Object::Object() :
	m_transform(Matrix4()) 
{
	m_owner = std::make_shared<SceneNode>(m_transform, this);
}

Object::Object(const Matrix4& transform) :
	m_transform(transform) 
{
	m_owner = std::make_shared<SceneNode>(m_transform, this);
}

Object::Object(const Object& other) :
	m_transform(other.m_transform) {}

void Object::SetTransform(const Matrix4& transform)
{
	m_transform = transform;
	m_owner->MarkDirty();
}

Matrix4 Object::GetTransform()
{
	return m_transform;
}

const Matrix4 Object::GetTransform() const
{
	return m_transform;
}