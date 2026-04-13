#include "sceneGraph.h"
#include <../../Resources/Header/Object.h>

using namespace Apex::SceneGraph;

SceneNode::SceneNode() :
	m_object(nullptr) {}

SceneNode::SceneNode(Object* object) :
	m_object(object) {}

std::shared_ptr<SceneNode> const& SceneNode::GetParent() const
{
	return m_parent;
}

Object* SceneNode::GetParentObj() const
{
	return m_parent ? m_parent->m_object : nullptr;
}

const vector<shared_ptr<SceneNode>>& SceneNode::GetChildren() const
{
	return m_children;
}

Object* Apex::SceneGraph::SceneNode::GetObject() const
{
	return m_object;
}

bool SceneNode::IsDescendantOf(std::shared_ptr<SceneNode> const& otherNode) const
{
	std::shared_ptr<SceneNode> current = m_parent;

	while (current.get() != nullptr)
	{
		if (current == otherNode) return true;

		current = current->m_parent;
	}

	return false;
}

bool Apex::SceneGraph::SceneNode::IsDescendantOf(SceneNode* otherNode) const
{
	std::shared_ptr<SceneNode> current = m_parent;

	while (current.get() != nullptr)
	{
		if (current.get() == otherNode) return true;

		current = current->m_parent;
	}

	return false;
}

void SceneNode::AddChild(shared_ptr<SceneNode> child)
{
	if (!child || child->m_parent.get() == this) return;
	// Detach from previous parent if it is different from this
	if (child->m_parent)
		child->m_parent->DetachChild(child.get());

	child->m_parent = shared_from_this();
	m_children.push_back(child);
	MarkDirty();
}

void SceneNode::DestroyChild(std::shared_ptr<SceneNode> const& child)
{
	//search the child in all the children
	auto iterator = std::find_if(m_children.begin(), m_children.end(),
		[child](const shared_ptr<SceneNode>& ptr)
		{
			return ptr == child;
		});

	if (iterator != m_children.end())
	{
		//'erase' deletes the entire subtree
		m_children.erase(iterator); 
	}
}

shared_ptr<SceneNode> SceneNode::DetachChild(SceneNode* child)
{
	auto it = std::find_if(m_children.begin(), m_children.end(),
		[&](const shared_ptr<SceneNode>& c) { return c.get() == child; });

	if (it == m_children.end()) return nullptr;

	shared_ptr<SceneNode> ptr = *it;
	ptr->m_parent = nullptr;
	m_children.erase(it);
	return ptr;
}

void SceneNode::Destroy()
{
	m_children.clear();	//all descandantes are also Destroyed
}

void SceneNode::SetParent(std::shared_ptr<SceneNode> const& newParent)
{
	if (!newParent)
	{
		m_parent->DetachChild(this);
		m_parent = nullptr;
		return;
	}

	if (m_parent == newParent ||	//No move to origine place
		newParent.get() == this ||	//No move to itself
		IsDescendantOf(newParent))	//No loop descendance
	{
		return;
	}

	shared_ptr<SceneNode> self = m_parent ? m_parent->DetachChild(this) : shared_from_this();
	newParent->AddChild(self);
	MarkDirty();
}

void Apex::SceneGraph::SceneNode::SetParent(SceneNode* newParent)
{
	if (!newParent && !m_parent)
	{
		m_parent->DetachChild(this);
		m_parent = nullptr;
		return;
	}

	if (m_parent.get() == newParent ||	//No move to origine place
		newParent == this ||	//No move to itself
		IsDescendantOf(newParent))	//No loop descendance
	{
		return;
	}

	shared_ptr<SceneNode> self = m_parent ? m_parent->DetachChild(this) : shared_from_this();
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

void SceneNode::InsertBefore(std::shared_ptr<SceneNode> const& other)
{
	if (!other || other.get() == this || IsDescendantOf(other))
		return;

	std::shared_ptr<SceneNode> const& parent = other->m_parent;

	if (!parent)
		return;

	// detach self
	shared_ptr<SceneNode> self =
		m_parent ? m_parent->DetachChild(this) : shared_from_this();

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

void SceneNode::InsertAfter(std::shared_ptr<SceneNode> const& other)
{
	if (!other || other.get() == this || IsDescendantOf(other))
		return;

	std::shared_ptr<SceneNode> const& parent = other->m_parent;

	if (!parent)
		return;

	shared_ptr<SceneNode> self =
		m_parent ? m_parent->DetachChild(this) : shared_from_this();

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