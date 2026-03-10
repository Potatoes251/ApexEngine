#include "Object.h"
#include "../../LowRenderer/header/SceneGraph.h"

using namespace Apex::Data;
using namespace LibMath;
using namespace Apex::SceneGraph;
	
size_t Apex::Data::Object::m_nextId = 0;

Apex::Data::Object::Object() :
	m_localTransform(Transform()), m_globalTransform(Transform())
{
	m_owner = std::make_shared<SceneNode>(this);
	m_id = m_nextId++;
	m_name = "Object" + std::to_string(m_id);
}

Object::Object(const Transform& localTransform, const Transform& globalTransform) :
	m_localTransform(localTransform), m_globalTransform(globalTransform)
{
	m_owner = std::make_shared<SceneNode>(this);
	m_id = m_nextId++;
	m_name = "Object" + std::to_string(m_id);
}

Object::Object(const Object& other) :
	m_localTransform(other.m_localTransform), m_globalTransform(other.m_globalTransform) 
{
	m_id = m_nextId++;
	m_name = "Object" + std::to_string(m_id);
}

void Object::SetLocalTransform(const Transform& transform)
{
	m_localTransform = transform;
	m_owner->MarkDirty();
}

void Apex::Data::Object::SetGlobalTransform(const Transform& transform)
{
	m_globalTransform = transform;
	m_owner->MarkDirty();
}

Transform& Object::GetLocalTransform()
{
	return m_localTransform;
}

const Transform& Object::GetLocalTransform() const
{
	return m_localTransform;
}

Transform& Object::GetGlobalTransform()
{
	return m_globalTransform;
}

const Transform& Object::GetGlobalTransform() const
{
	return m_globalTransform;
}

void Object::Start()
{
	for (auto& comp : m_components)
		if (comp->IsEnabled())
			comp->OnStart();
}

void Object::Update(float deltaTime)
{
	for (auto& comp : m_components)
	{
		if (comp->IsEnabled())
			comp->OnUpdate(deltaTime);
	}
	auto pos = m_localTransform.getPosition();
}