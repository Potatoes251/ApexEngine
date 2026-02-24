#ifndef SCENE_GRAPH
#define SCENE_GRAPH

#include <vector>
#include <LibMath/Matrix/Matrix4.h>

using std::vector;
using std::unique_ptr;
using LibMath::Matrix4;

namespace Apex::SceneGraph
{
	class SceneNode
	{
	public:
		SceneNode(void* model, Matrix4 localTransform = Matrix4());
		~SceneNode() = default;

		Matrix4&	GetLocalTransform();
		void		SetLocalTransform(const Matrix4& localTransform);

		Matrix4		GetWorldTransform();
		void		SetWorldTransform();

		//get model
		//set model

		void		AddChild(unique_ptr<SceneNode> child);

	private:
		Matrix4							m_localTransform;
		Matrix4							m_worldTransform;
		bool							m_dirty = true;

		void*							m_model;

		SceneNode*						m_parent = nullptr;	
		vector<unique_ptr<SceneNode>>	m_children;
		
		void		MarkDirty();
	};
}
#endif