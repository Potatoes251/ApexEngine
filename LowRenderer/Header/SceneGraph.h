#ifndef SCENE_GRAPH
#define SCENE_GRAPH

#include <vector>
#include <memory>

namespace Apex::Data { class Object; }

using std::vector;
using std::shared_ptr;
using Apex::Data::Object;

namespace Apex::SceneGraph
{
	class SceneNode : public std::enable_shared_from_this<SceneNode>
	{
	public:
		SceneNode();
		SceneNode(Object* object = nullptr);
		SceneNode(SceneNode& other) = delete;
		~SceneNode() = default;

				SceneNode&						operator=(const SceneNode& other) = delete;

				SceneNode*						GetParent() const;
		const	vector<shared_ptr<SceneNode>>&	GetChildren() const;
				Object*							GetObject() const;

				void							AddChild(shared_ptr<SceneNode> child);
				void							DestroyChild(SceneNode* child);
				shared_ptr<SceneNode>			DetachChild(SceneNode* child);			//return a unique pointer, to use children use ' .get() ' on each children

				void							Destroy();
				void							SetParent(SceneNode* newParent);
				void							MarkDirty();
				
				bool							IsDescendantOf(SceneNode& otherNode);

				void							InsertBefore(SceneNode* other);
				void							InsertAfter(SceneNode* other);
	private:

		bool							m_dirty = true;

		Object*							m_object;

		SceneNode*						m_parent = nullptr;	
		vector<shared_ptr<SceneNode>>	m_children;
	};
}

#endif