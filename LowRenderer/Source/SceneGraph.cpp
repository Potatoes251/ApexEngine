#include "sceneGraph.h"
#include <../../Resources/Header/Object.h>

using namespace Apex::SceneGraph;

SceneNode::SceneNode() :
	m_object(nullptr) {}

SceneNode::SceneNode(Object* object) :
	m_object(object) {}

SceneNode* SceneNode::GetParent() const
{
	return m_parent;
}

const vector<shared_ptr<SceneNode>>& SceneNode::GetChildren() const
{
	return m_children;
}

Object* Apex::SceneGraph::SceneNode::GetObject() const
{
	return m_object;
}

bool SceneNode::IsDescendantOf(SceneNode& otherNode)
{
	SceneNode* current = m_parent;

	while (current != nullptr)
	{
		if (current == &otherNode) return true;

		current = current->m_parent;
	}

	return false;
}

void SceneNode::AddChild(shared_ptr<SceneNode> child)
{
	if (!child) return;
	// Detach from previous parent if it is different from this
	if (child->m_parent && child->m_parent != this)
		child->m_parent->DetachChild(child.get());
	else if (child->m_parent == this)
		return;  // already a child of this node

	child->m_parent = this;
	m_children.push_back(child);
	MarkDirty();
}

void SceneNode::DestroyChild(SceneNode* child)
{
	//search the child in all the children
	auto iterator = std::find_if(m_children.begin(), m_children.end(),
		[child](const shared_ptr<SceneNode>& ptr)
		{
			return ptr.get() == child;
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

void SceneNode::SetParent(SceneNode* newParent)
{
	if (m_parent == newParent	||	//No move to origine place
		newParent == this		||	//No move to itself
		IsDescendantOf(*newParent))	//No loop descendance
	{
		return;
	}

	if (!newParent)
	{
		m_parent->DetachChild(this);
		m_parent = nullptr;
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

void SceneNode::InsertBefore(SceneNode* other)
{
	if (!other || other == this)
		return;

	SceneNode* parent = other->m_parent;

	if (!parent)
		return;

	if (IsDescendantOf(*other))
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
			return ptr.get() == other;
		});

	if (it != siblings.end())
	{
		m_parent = parent;
		siblings.insert(it, self);
	}

	MarkDirty();
}

void SceneNode::InsertAfter(SceneNode* other)
{
	if (!other || other == this)
		return;

	SceneNode* parent = other->m_parent;

	if (!parent)
		return;

	if (IsDescendantOf(*other))
		return;

	shared_ptr<SceneNode> self =
		m_parent ? m_parent->DetachChild(this) : shared_from_this();

	auto& siblings = parent->m_children;

	auto it = std::find_if(
		siblings.begin(),
		siblings.end(),
		[&](const shared_ptr<SceneNode>& ptr)
		{
			return ptr.get() == other;
		});

	if (it != siblings.end())
	{
		++it;

		m_parent = parent;
		siblings.insert(it, self);
	}

	MarkDirty();
}