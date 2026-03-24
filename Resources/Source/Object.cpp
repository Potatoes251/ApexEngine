#include "Object.h"

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

std::unique_ptr<Object> Object::Clone()
{
	std::unique_ptr<Object> clone = std::make_unique<Object>();
	clone->SetName(m_name);
	clone->SetLocalTransform(m_localTransform);

	for (auto* comp : GetComponents())
	{
		std::unique_ptr<Component> newComp = comp->Clone();
		clone->AddComponentDirect(std::move(newComp));
	}

	std::shared_ptr<SceneNode> cloneNode = clone->GetSceneNode();
	for (auto& child : m_owner->GetChildren())
	{
		std::unique_ptr<Object> childClone = child->GetObject()->Clone();
		if (childClone)
			cloneNode->AddChild(childClone->GetSceneNode());
	}

	return clone;
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

const Transform& Object::GetLocalTransform() const
{
	return m_localTransform;
}

const Transform& Object::GetGlobalTransform()
{
	if (m_owner && m_owner->IsDirty()) UpdateGlobalTransform();

	return m_globalTransform;
}

void Object::UpdateGlobalTransform()
{
	if (!m_owner || !m_owner->IsDirty()) return;

	Object* parent = m_owner->GetParentObj();

	if (!parent) 
	{
		m_globalTransform = m_localTransform;
		m_owner->CleanDirty();
		return;
	}

	m_globalTransform = parent->GetGlobalTransform() * m_localTransform;
	m_owner->CleanDirty();
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

void Object::AddComponentDirect(std::unique_ptr<Component> comp)
{
	comp->m_owner = this;
	m_components.push_back(std::move(comp));

	if (m_components.size() == 1)
		m_components.back()->SetId(0);
	else
	{
		m_components.back()->SetId(m_components[m_components.size() - 2]->GetId() + 1);
	}
}