#include "Object.h"

#include "../../Physic/Header/Rigidbody.h"

using namespace Apex::Data;
using namespace LibMath;
using namespace Apex::SceneGraph;
using namespace Apex;
	
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

		if (!newComp)
		{
			std::cerr << "Clone returned nullptr for component: " << comp->GetTypeName() << std::endl;
			continue;
		}

		clone->AddComponentDirect(std::move(newComp));
	}

	std::shared_ptr<SceneNode> cloneNode = clone->GetSceneNode();
	for (auto& child : m_owner->GetChildren())
	{
		std::unique_ptr<Object> childClone = child->GetObject()->Clone();
		if (childClone)
			cloneNode->AddChild(childClone->GetSceneNode());
	}

	clone->SetOnChangedCallback(m_onChanged);

	return clone;
}

void Object::SetLocalTransform(const Transform& transform)
{
	m_localTransform = transform;
	m_owner->MarkDirty();

	OnChanged();
}

void Object::SetLocalPosition(const LibMath::Vector3& position)
{
	m_localTransform.setPosition(position);
	m_owner->MarkDirty();
}

void Object::SetLocalRotation(const LibMath::Quaternion& rotation)
{
	m_localTransform.setRotation(rotation);
	m_owner->MarkDirty();
}

void Object::SetLocalScale(const LibMath::Vector3& scale)
{
	m_localTransform.setScale(scale);
	m_owner->MarkDirty();
}

void Apex::Data::Object::SetGlobalTransform(const Transform& transform)
{
	m_globalTransform = transform;
	m_owner->MarkDirty();

	OnChanged();
}

void Apex::Data::Object::SetName(std::string name)
{
	m_name = name;
	OnChanged();
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

void Object::FixedUpdate(float deltaTime_s)
{
	for (auto& comp : m_components)
	{
		if (comp->IsEnabled())
			comp->OnFixedUpdate(deltaTime_s);
	}
}

void Object::Update(float deltaTime_s)
{
	for (auto& comp : m_components)
	{
		if (comp->IsEnabled())
			comp->OnUpdate(deltaTime_s);
	}
}

void Object::LateUpdate(float deltaTime_s)
{
	for (auto& comp : m_components)
	{
		if (comp->IsEnabled())
			comp->OnLateUpdate(deltaTime_s);
	}
}

void Object::OnTriggerEnter(Physic::Collider* other)
{
	for (auto& comp : m_components)
	{
		if (comp->IsEnabled())
			comp->OnTriggerEnter(other);
	}
}

void Object::OnTriggerExit(Physic::Collider* other)
{
	for (auto& comp : m_components)
	{
		if (comp->IsEnabled())
			comp->OnTriggerExit(other);
	}
}

void Object::AddTag(std::string tag)
{
	m_tags.insert(std::move(tag));
}

void Object::RemoveTag(std::string const& tag)
{
	m_tags.erase(tag);
}

bool Object::HasTag(std::string const& tag) const
{
	return m_tags.contains(tag);
}

std::unordered_set<std::string> const& Apex::Data::Object::GetTags() const
{
	return m_tags;
}

void Object::AddComponentDirect(std::unique_ptr<Component> comp)
{
	comp->m_owner = this;
	m_components.push_back(std::move(comp));

	if (m_components.size() == 1)
		m_components.back()->SetId(0);
	else
		m_components.back()->SetId(m_components[m_components.size() - 2]->GetId() + 1);

	OnChanged();
}

void Object::RemoveComponent(Component* compToRemove, Apex::Physic::PhysicSystem& phys)
{
	for (size_t i = 0; i < m_components.size(); i++)
	{
		if (m_components[i].get() == compToRemove)
		{
			auto* rb = dynamic_cast<Apex::Physic::RigidBodyComponent*>(m_components[i].get());

			if (rb)
				phys.RemoveActor(*rb);

			m_components.erase(m_components.begin() + i);

			OnChanged();

			compToRemove->m_owner = nullptr;
			return;
		}
	}
}

void Object::SetOnChangedCallback(std::function<void()> callback)
{
	m_onChanged = std::move(callback);
}

void Object::OnChanged()
{
	if (m_onChanged)
		m_onChanged();
}
