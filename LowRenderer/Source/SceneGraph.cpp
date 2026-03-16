#include "sceneGraph.h"
#include <../../Resources/header/Object.h>

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
	std::cout << "\n[AddChild] Called\n";
	std::cout << "Parent: " << this << "\n";
	std::cout << "Child : " << child.get() << "\n";
	std::cout << "Children count BEFORE: "
		<< m_children.size() << "\n";

	//remove from parent
	if (child->m_parent)
	{
		std::cout << "Child already has parent: "
			<< child->m_parent << "\n";

		if (child->m_parent != this)
		{
			std::cout << "Detaching from old parent...\n";
			shared_ptr<SceneNode> detached =
				child->m_parent->DetachChild(child.get());

			if (detached)
				std::cout << "Detach from old parent SUCCESS\n";
			else
				std::cout << "Detach from old parent FAILED\n";
		}
		else
		{
			std::cout << "Child already attached to this parent\n";
		}
	}

	child->m_parent = this;
	m_children.push_back(child);

	std::cout << "Child attached to new parent\n";

	MarkDirty();

	std::cout << "Children count AFTER: "
		<< m_children.size() << "\n";
	std::cout << "[AddChild] SUCCESS\n";
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
	std::cout << "\n[DetachChild] Called\n";
	std::cout << "Parent: " << this << "\n";
	std::cout << "Child to detach: " << child << "\n";
	std::cout << "Children count BEFORE: " << m_children.size() << "\n";

	//search the child in all the children
	auto iterator = std::find_if(m_children.begin(), m_children.end(),
		[&](const shared_ptr<SceneNode>& c) 
		{ 
			return c.get() == child; 
		});

	if (iterator != m_children.end())
	{
		std::cout << "Child found. Detaching...\n";

		shared_ptr<SceneNode> ptr = *iterator;
		ptr->m_parent = nullptr;
		m_children.erase(iterator);

		std::cout << "Children count AFTER: " << m_children.size() << "\n";
		std::cout << "[DetachChild] SUCCESS\n";

		return ptr;
	}

	std::cout << "Child NOT found.\n";
	return nullptr;
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