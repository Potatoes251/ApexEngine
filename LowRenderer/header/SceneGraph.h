#ifndef SCENE_GRAPH
#define SCENE_GRAPH

#include <vector>
#include <LibMath/Matrix/Matrix4.h>

using std::vector;
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

		//get model
		//set model



	private:
		Matrix4				m_localTransform;
		Matrix4				m_worldTransform;

		void*				m_model;

		SceneNode*			m_parent;	
		vector<SceneNode*>	m_children;
	};
}
#endif