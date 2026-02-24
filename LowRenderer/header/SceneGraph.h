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
		SceneNode(Matrix4 localTransform = Matrix4());
		SceneNode(SceneNode& other) = delete;
		~SceneNode() = default;

				Matrix4&						GetLocalTransform();
				void							SetLocalTransform(const Matrix4& localTransform);

		const	Matrix4							GetWorldTransform();
				void							SetWorldTransform();

				SceneNode*						GetParent() const;
		const	vector<unique_ptr<SceneNode>>&	GetChildren() const;

				void							AddChild(unique_ptr<SceneNode> child);
				void							DestroyChild(SceneNode* child);
				unique_ptr<SceneNode>			DetachChild(SceneNode* child);			//return a unique pointer, to use children use ' .get() ' on each children

				void							Destroy();
	private:
		Matrix4							m_localTransform;
		Matrix4							m_worldTransform;
		bool							m_dirty = true;

		SceneNode*						m_parent = nullptr;	
		vector<unique_ptr<SceneNode>>	m_children;
		
		void	MarkDirty();
	};
}
#endif