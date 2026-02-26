#include "sceneGraph.h"
#include <../../Resources/header/Object.h>

using namespace Apex::SceneGraph;

SceneNode::SceneNode(Matrix4 localTransform) :
	m_worldTransform(localTransform), m_object(nullptr) {}

SceneNode::SceneNode(Matrix4 localTransform, Object* object) :
	m_worldTransform(localTransform), m_object(object) {}

const Matrix4 SceneNode::GetWorldTransform()
{
	if (m_dirty)
	{
		SetWorldTransform();
	}
	return m_worldTransform;
}

void SceneNode::SetWorldTransform()
{
	if (!m_dirty)
	{
		return;
	}
	if (m_parent)
	{
		m_worldTransform = m_parent->GetWorldTransform() * m_object->GetTransform();
	}
	else
	{
		m_worldTransform = m_object->GetTransform();
	}

	m_dirty = false;
}

SceneNode* SceneNode::GetParent() const
{
	return m_parent;
}

const vector<shared_ptr<SceneNode>>& SceneNode::GetChildren() const
{
	return m_children;
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
	//remove from parent
	if (child->m_parent && child->m_parent != this)
	{
		shared_ptr<SceneNode> detached = child->m_parent->DetachChild(child.get());
	}

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
	//search the child in all the children
	auto iterator = std::find_if(m_children.begin(), m_children.end(),
		[&](const shared_ptr<SceneNode>& c) 
		{ 
			return c.get() == child; 
		});

	if (iterator != m_children.end())
	{
		shared_ptr<SceneNode> ptr = *iterator;
		ptr->m_parent = nullptr;
		m_children.erase(iterator);
		return ptr;
	}
	return nullptr;
}

void SceneNode::Destroy()
{
	m_children.clear();	//all descandantes are also Destroyed
}

void SceneNode::Move(SceneNode& newParent)
{
	if (m_parent == &newParent	||	//No move to origine place
		&newParent == this		||	//No move to itself
		IsDescendantOf(newParent))	//No loop descendance
	{
		return;
	}

	shared_ptr<SceneNode> self = m_parent ? m_parent->DetachChild(this) : shared_from_this();
	newParent.AddChild(self);
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