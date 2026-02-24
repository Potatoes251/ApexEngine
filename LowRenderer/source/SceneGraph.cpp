#include "sceneGraph.h"

using namespace Apex::SceneGraph;

SceneNode::SceneNode(void* model, Matrix4 localTransform) :
	m_model(model), m_localTransform(localTransform), m_worldTransform(localTransform) {}

Matrix4& SceneNode::GetLocalTransform()
{
	return m_localTransform;
}

void SceneNode::SetLocalTransform(const Matrix4& localTransform)
{
	m_localTransform = localTransform;
	MarkDirty();
}

Matrix4 SceneNode::GetWorldTransform()
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

void SceneNode::MarkDirty()
{
	m_dirty = true;
	for (unique_ptr<SceneNode>& child : m_children)
	{
		child->MarkDirty();
	}
}

void SceneNode::AddChild(unique_ptr<SceneNode> child)
{
	m_children.push_back(std::move(child));
	child->m_parent = this;
}
