#include "SceneGraph.h"
#include "../../Resources/Header/Object.h"

using namespace Apex::SceneGraph;
using namespace Apex::Data;

SceneNode::SceneNode() :
	m_object(nullptr) {}

SceneNode::SceneNode(Object* object) :
	m_object(object) {}

shared_ptr<SceneNode> SceneNode::GetParent() const
{
	return m_parent.lock();
}

Object* SceneNode::GetParentObj() const
{
	if (auto parent = m_parent.lock())
	{
		return parent->m_object;
	}
	return nullptr;
}

const vector<shared_ptr<SceneNode>>& SceneNode::GetChildren() const
{
	return m_children;
}

Object* SceneNode::GetObject() const
{
	return m_object;
}

bool SceneNode::IsDescendantOf(shared_ptr<SceneNode> const& otherNode) const
{
	shared_ptr<SceneNode> current = m_parent.lock();

	while (current)
	{
		if (current == otherNode) return true;

		current = current->m_parent.lock();
	}

	return false;
}

bool SceneNode::IsDescendantOf(SceneNode* otherNode) const
{
	shared_ptr<SceneNode> current = m_parent.lock();

	while (current.get() != nullptr)
	{
		if (current.get() == otherNode) return true;

		current = current->m_parent.lock();
	}

	return false;
}

void SceneNode::AddChild(shared_ptr<SceneNode> child)
{
	if (!child) return;

	auto currentParent = child->m_parent.lock();

	if (currentParent && currentParent.get() == this)
	{
		return;
	}

	if (currentParent)
	{
		currentParent->DetachChild(child.get());
	}

	child->m_parent = shared_from_this();
	m_children.push_back(child);

	MarkDirty();
}

shared_ptr<SceneNode> SceneNode::DetachChild(SceneNode* child)
{
	auto it = std::find_if(m_children.begin(), m_children.end(),
		[&](const shared_ptr<SceneNode>& c) { return c.get() == child; });

	if (it == m_children.end()) return nullptr;

	shared_ptr<SceneNode> ptr = *it;
	ptr->m_parent.reset();
	m_children.erase(it);
	return ptr;
}

void SceneNode::Destroy()
{
	for (auto& child : m_children)
	{
		child->Destroy();
	}

	m_children.clear();
	m_object = nullptr;

	shared_ptr<SceneNode> parent = m_parent.lock();
	if (parent)
	{
		parent->DetachChild(this);
	}
}

void SceneNode::SetParent(shared_ptr<SceneNode> const& newParent)
{
	shared_ptr<SceneNode> parent = m_parent.lock();

	if (!newParent)
	{
		parent->DetachChild(this);
		parent = nullptr;
		return;
	}

	if (parent == newParent ||	//No move to origine place
		newParent.get() == this ||	//No move to itself
		IsDescendantOf(newParent))	//No loop descendance
	{
		return;
	}

	shared_ptr<SceneNode> self = parent ? parent->DetachChild(this) : shared_from_this();
	newParent->AddChild(self);
	MarkDirty();
}

void SceneNode::SetParent(SceneNode* newParent)
{
	auto currentParent = m_parent.lock();

	if (!newParent)
	{
		if (currentParent)
		{
			currentParent->DetachChild(this);
		}
		m_parent.reset();
		return;
	}

	if ((currentParent && currentParent.get() == newParent) || // fix
		newParent == this ||
		IsDescendantOf(newParent))
	{
		return;
	}

	shared_ptr<SceneNode> self =
		currentParent ? currentParent->DetachChild(this)
		: shared_from_this();

	newParent->AddChild(self);
	MarkDirty();
}

void SceneNode::MarkDirty()
{
	m_dirty = true;
	for (shared_ptr<SceneNode>& child : m_children)
	{
		child->MarkDirty();
	}
}

void SceneNode::InsertBefore(shared_ptr<SceneNode> const& other)
{
	if (!other || other.get() == this || IsDescendantOf(other))
		return;

	shared_ptr<SceneNode> const& parent = other->m_parent.lock();
	if (!parent) return;

	// detach self
	shared_ptr<SceneNode> currentParent = m_parent.lock();
	shared_ptr<SceneNode> self = shared_from_this();
	if (currentParent)
	{
		self = currentParent->DetachChild(self.get());
	}

	auto& siblings = parent->m_children;

	auto it = std::find_if(
		siblings.begin(),
		siblings.end(),
		[&](const shared_ptr<SceneNode>& ptr)
		{
			return ptr == other;
		});

	if (it != siblings.end())
	{
		m_parent = parent;
		siblings.insert(it, self);
	}

	MarkDirty();
}

void SceneNode::InsertAfter(shared_ptr<SceneNode> const& other)
{
	if (!other || other.get() == this || IsDescendantOf(other))
		return;

	shared_ptr<SceneNode> const& parent = other->m_parent.lock();
	if (!parent) return;

	shared_ptr<SceneNode> currentParent = m_parent.lock();
	shared_ptr<SceneNode> self = shared_from_this();
	if (currentParent)
	{
		self = currentParent->DetachChild(self.get());
	}

	auto& siblings = parent->m_children;

	auto it = std::find_if(
		siblings.begin(),
		siblings.end(),
		[&](const shared_ptr<SceneNode>& ptr)
		{
			return ptr == other;
		});

	if (it != siblings.end())
	{
		++it;

		m_parent = parent;
		siblings.insert(it, self);
	}

	MarkDirty();
}

bool SceneNode::IsDirty() const
{
	return m_dirty;
}

void SceneNode::CleanDirty()
{
	m_dirty = false;
}