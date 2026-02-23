#include "sceneGraph.h"

using namespace Apex::SceneGraph;

SceneNode::SceneNode(void* model, Matrix4 localTransform) :
	m_model(model), m_localTransform(localTransform) {}

Matrix4& Apex::SceneGraph::SceneNode::GetLocalTransform()
{
	return m_localTransform;
}

void Apex::SceneGraph::SceneNode::SetLocalTransform(const Matrix4& localTransform)
{
	m_localTransform = localTransform;
}
