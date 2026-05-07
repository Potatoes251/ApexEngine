#include "Object.h"

#include "../../Physic/Header/Rigidbody.h"

#include "Log.h"

using namespace Apex::Data;
using namespace LibMath;
using namespace Apex::SceneGraph;	

Apex::Data::Object::Object()
{
	m_owner = std::make_shared<SceneNode>(this);
	m_id = m_nextId++;
	m_name = "Object" + std::to_string(m_id);
}

Apex::Data::Object::Object(size_t forcedId)
{
	m_owner = std::make_shared<SceneNode>(this);
	
	m_id = forcedId;

	if (forcedId != INVALID_ID && m_nextId <= forcedId)
		m_nextId = ++forcedId;

	m_name = "Object" + std::to_string(m_id);
}

Object::Object(const Transform& localTransform, const Transform& globalTransform) :
	m_localTransform(localTransform), m_globalTransform(globalTransform)
{
	m_owner = std::make_shared<SceneNode>(this);
	m_id = m_nextId++;
	m_name = "Object" + std::to_string(m_id);
}

Apex::Data::Object::~Object()
{
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

void Object::SetId(size_t id)
{
	if (id == INVALID_ID) return;

	m_id = id;

	if (m_nextId <= id)
		m_nextId = ++id;
}

void Object::SetCompId(size_t oldId, size_t newId)
{
	if (oldId == newId || newId == INVALID_ID) return;

	Component* comp = GetComponent(oldId);

	if (!comp) return;

	if (oldId != INVALID_ID)
		m_freeCompIds.insert(oldId);

	comp->SetId(newId);
	m_freeCompIds.erase(newId);

	if (m_nextCompId <= newId)
		m_nextCompId = ++newId;
}

void Object::SetCompId(Component* comp, size_t newId)
{
	if (!comp || newId == INVALID_ID) return;

	if (comp->GetId() != INVALID_ID)
		m_freeCompIds.insert(comp->GetId());

	comp->SetId(newId);
	m_freeCompIds.erase(newId);

	if (m_nextCompId <= newId)
		m_nextCompId = ++newId;
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

void Object::RemoveCompId(size_t oldId)
{
	if (oldId == INVALID_ID) return;

	Component* comp = GetComponent(oldId);

	if (!comp) return;

	m_freeCompIds.insert(comp->GetId());
	comp->SetId(INVALID_ID);
}

void Object::RemoveCompId(Component* comp)
{
	if (!comp || comp->GetId() == INVALID_ID) return;

	m_freeCompIds.insert(comp->GetId());
	comp->SetId(INVALID_ID);
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
	m_name = name;
	OnChanged();
}

const Transform& Object::GetLocalTransform() const
{
	return m_localTransform;
}

const Transform& Object::GetGlobalTransform() const
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

	if (m_freeCompIds.empty())
	{
		m_components.back()->SetId(m_nextCompId++);
	}
	else
	{
		m_components.back()->SetId(*m_freeCompIds.begin());
		m_freeCompIds.erase(m_freeCompIds.begin());
	}

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

			m_freeCompIds.insert(compToRemove->GetId());

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

			m_freeCompIds.insert(compId);

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
