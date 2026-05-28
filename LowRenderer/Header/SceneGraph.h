#ifndef SCENE_GRAPH
#define SCENE_GRAPH

#include <vector>
#include <memory>

namespace Apex::Data { class Object; }

using std::vector;
using std::shared_ptr;
using std::weak_ptr;
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

		const	vector<shared_ptr<SceneNode>>&	GetChildren() const;
		std::shared_ptr<SceneNode> GetParent() const;
		Object*		GetParentObj() const;
		Object*		GetObject() const;

		shared_ptr<SceneNode>	DetachChild(SceneNode* child);
		void		AddChild(shared_ptr<SceneNode> child);

		void		Destroy();
		void		SetParent(shared_ptr<SceneNode> const& newParent);
		void		SetParent(SceneNode* newParent);
		void		MarkDirty();
		bool		IsDirty() const;
		void		CleanDirty();
		
		bool		IsDescendantOf(shared_ptr<SceneNode> const& otherNode) const;
		bool		IsDescendantOf(SceneNode* otherNode) const;

		void		InsertBefore(shared_ptr<SceneNode> const& other);
		void		InsertAfter(shared_ptr<SceneNode> const& other);
	private:

		mutable bool					m_dirty = true;

		Object*							m_object;

		weak_ptr<SceneNode>				m_parent;
		vector<shared_ptr<SceneNode>>	m_children;
	};
}

#endif