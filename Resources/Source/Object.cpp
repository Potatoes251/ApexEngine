#include "Object.h"

#include "../../Physic/Header/Rigidbody.h"
#include "../../Physic/Header/Physic.h"

#include "Log.h"

using namespace Apex::Data;
using namespace Apex::SceneGraph;	

Apex::Data::Object::Object()
{
	m_owner = std::make_shared<SceneNode>(this);
	m_id = m_nextId++;
	m_name = GenerateUniqueName();
	RegisterName(m_name);
}

Apex::Data::Object::Object(size_t forcedId)
{
	m_owner = std::make_shared<SceneNode>(this);
	
	m_id = forcedId;

	if (forcedId != INVALID_ID && m_nextId <= forcedId)
		m_nextId = ++forcedId;

	m_name = GenerateUniqueName();
	RegisterName(m_name);
}

Object::Object(const LibMath::Transform& localTransform, const LibMath::Transform& globalTransform) :
	m_localTransform(localTransform), m_globalTransform(globalTransform)
{
	m_owner = std::make_shared<SceneNode>(this);
	m_id = m_nextId++;
	m_name = GenerateUniqueName();
	RegisterName(m_name);
}

Apex::Data::Object::~Object()
{
	UnregisterName(m_name);
}

std::unique_ptr<Object> Object::Clone()
{
	std::unique_ptr<Object> clone = std::make_unique<Object>(m_id);
	clone->SetName(m_name);
	clone->SetLocalTransform(m_localTransform);

	for (auto* comp : GetComponents())
	{
		std::unique_ptr<Component> newComp = comp->Clone();

		if (!newComp)
		{
			LOG_ERROR("Clone returned nullptr for component: {}", comp->GetTypeName());
			continue;
		}

		clone->AddComponentDirect(std::move(newComp));
	}

	clone->SetOnChangedCallback(m_onChanged);

	return clone;
}

void Object::SetId(size_t id)
{
	if (id == INVALID_ID) return;

	m_id = id;

	if (m_nextId <= id)
		m_nextId = ++id;
}

size_t Object::NewId()
{
	m_id = m_nextId++;
	return m_id;
}

void Object::RemoveId()
{
	m_id = INVALID_ID;
}

void Object::SetLocalTransform(const LibMath::Transform& transform)
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

void Apex::Data::Object::SetGlobalTransform(const LibMath::Transform& transform)
{	
	Object* parent = GetParent();
	
	if (!parent)
	{
		m_localTransform = transform;
	}
	else
	{
		m_localTransform = parent->GetGlobalTransform().inversed() * transform;
	}

	m_owner->MarkDirty();
	OnChanged();

	UpdateGlobalTransform();
}

void Apex::Data::Object::SetName(std::string name)
{
	if (m_name != name)
	{
		UnregisterName(m_name);
		RegisterName(name);
		m_name = name;
		OnChanged();
	}
}

const LibMath::Transform& Object::GetLocalTransform() const
{
	return m_localTransform;
}

const LibMath::Transform& Object::GetGlobalTransform() const
{
	if (m_owner && m_owner->IsDirty()) UpdateGlobalTransform();

	return m_globalTransform;
}

void Object::UpdateGlobalTransform() const
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

Object* Object::GetParent() const
{
	auto parentNode = m_owner->GetParent();

	if (!parentNode) return nullptr;

	return parentNode->GetObject();
}

void Object::Start()
{
	for (auto& comp : m_components)
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

size_t Object::AddComponentDirect(std::unique_ptr<Component> comp)
{
	comp->m_owner = this;
	m_components.push_back(std::move(comp));

	m_components.back()->SetId(m_nextCompId++);

	OnChanged();

	return m_components.back()->GetId();
}

std::vector<Apex::Component*> Object::GetComponents() const
{
	std::vector<Component*> comps;
	for (auto& c : m_components)
		comps.push_back(c.get());
	return comps;
}

Apex::Component* Object::GetComponent(size_t id)
{
	for (auto& comp : m_components)
	{
		if (comp && comp->GetId() == id)
		{
			return comp.get();
		}
	}
	return nullptr;
}

void Object::RemoveComponent(Component* compToRemove, Physic::PhysicSystem* phys)
{
	for (size_t i = 0; i < m_components.size(); i++)
	{
		if (m_components[i].get() == compToRemove)
		{
			Physic::RigidBodyComponent* rb = dynamic_cast<Physic::RigidBodyComponent*>(m_components[i].get());

			if (rb && phys)
				phys->RemoveActor(*rb);

			m_components.erase(m_components.begin() + i);

			OnChanged();

			compToRemove->m_owner = nullptr;
			return;
		}
	}
}

void Object::RemoveComponent(size_t compId, Physic::PhysicSystem* phys)
{
	for (size_t i = 0; i < m_components.size(); i++)
	{
		if (m_components[i] && m_components[i]->GetId() == compId)
		{
			m_components[i]->m_owner = nullptr;
			Physic::RigidBodyComponent* rb = dynamic_cast<Physic::RigidBodyComponent*>(m_components[i].get());

			if (rb && phys)
				phys->RemoveActor(*rb);

			m_components.erase(m_components.begin() + i);

			OnChanged();
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

std::string Object::GenerateUniqueName()
{
	constexpr const char* baseName = "Object";

	// First object keeps plain "Object"
	if (!m_usedNames.contains(baseName))
		return baseName;

	size_t index = 1;

	while (true)
	{
		std::string candidate = std::string(baseName) + std::to_string(index);

		if (!m_usedNames.contains(candidate))
			return candidate;

		++index;
	}
}

void Object::RegisterName(const std::string& name)
{
	m_usedNames.insert(name);
}

void Object::UnregisterName(const std::string& name)
{
	m_usedNames.erase(name);
}