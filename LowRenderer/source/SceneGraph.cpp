#include "sceneGraph.h"

using namespace Apex::SceneGraph;

SceneNode::SceneNode(Matrix4 localTransform) :
	m_localTransform(localTransform), m_worldTransform(localTransform) {}

Matrix4 SceneNode::GetLocalTransform()
{
	return m_localTransform;
}

const Matrix4 Apex::SceneGraph::SceneNode::GetLocalTransform() const
{
	return m_localTransform;
}

void SceneNode::SetLocalTransform(const Matrix4& localTransform)
{
	m_localTransform = localTransform;
	MarkDirty();
}

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
		m_worldTransform = m_parent->GetWorldTransform() * m_localTransform;
	}
	else
	{
		m_worldTransform = m_localTransform;
	}

	m_dirty = false;
}

SceneNode* SceneNode::GetParent() const
{
	return m_parent;
}

const vector<unique_ptr<SceneNode>>& SceneNode::GetChildren() const
{
	return m_children;
}

void SceneNode::MarkDirty()
{
	m_dirty = true;
	for (unique_ptr<SceneNode>& child : m_children)
	{
		child->MarkDirty();
	}
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

void SceneNode::AddChild(unique_ptr<SceneNode> child)
{
	if (child->m_parent != nullptr)
	{
		unique_ptr<SceneNode> detached = child->m_parent->DetachChild(child.get());

		child = std::move(detached);
	}

	child->m_parent = this;
	m_children.push_back(std::move(child));
}

void SceneNode::DestroyChild(SceneNode* child)
{
	//search the child in all the children
	auto iterator = std::find_if(m_children.begin(), m_children.end(),
		[child](const unique_ptr<SceneNode>& ptr)
		{
			return ptr.get() == child;
		});

	if (iterator != m_children.end())
	{
		//'erase' deletes the entire subtree
		m_children.erase(iterator); 
	}
}

unique_ptr<SceneNode> SceneNode::DetachChild(SceneNode* child)
{
	//search the child in all the children
	auto iterator = std::find_if(m_children.begin(), m_children.end(),
		[child](const unique_ptr<SceneNode>& ptr)
		{
			return ptr.get() == child;
		});

	if (iterator != m_children.end())
	{
		unique_ptr<SceneNode> result = std::move(*iterator);
		m_children.erase(iterator);
		result->m_parent = nullptr;
		return result;
	}

	return nullptr;
}

void SceneNode::Destroy()
{
	if (m_parent != nullptr)
	{
		m_parent->DestroyChild(this);
	}
}

void SceneNode::Move(SceneNode& newParent)
{
	if (m_parent == &newParent			||	//No move to origine place
		this == &newParent				||	//No move to itself
		newParent.IsDescendantOf(*this) ||	//No loop descendance
		m_parent == nullptr)				
	{
		return;
	}

	unique_ptr<SceneNode> self = m_parent->DetachChild(this);
	newParent.AddChild(std::move(self));

	MarkDirty();
}
