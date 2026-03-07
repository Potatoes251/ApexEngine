#ifndef OBJECT
#define OBJECT

#include <LibMath/Matrix/Matrix4.h>
#include "LibMath/Transform.h"

#include "../../Core/Header/Component.h"
#include "../../LowRenderer/Header/SceneGraph.h"

#include <vector>
#include <memory>
#include <utility>

namespace Apex::Data
{
	class Object
	{
	public:
		Object();
		Object(const LibMath::Transform& localTransform, const LibMath::Transform& globalTransform);
		Object(const Object& other);
		~Object() = default;

				Object&		operator=(const Object& other) = delete;

				LibMath::Transform	GetLocalTransform();
		const	LibMath::Transform	GetLocalTransform() const;
				LibMath::Transform	GetGlobalTransform();
		const	LibMath::Transform	GetGlobalTransform() const;

				void		SetLocalTransform(const LibMath::Transform& transform);
				void		SetGlobalTransform(const LibMath::Transform& transform);

				void		Start();
				void		Update(float deltaTime);

				std::shared_ptr<Apex::SceneGraph::SceneNode>	GetSceneNode() { return m_owner; }
				Apex::SceneGraph::SceneNode*					GetSceneNodeRaw() { return m_owner.get(); }

				template<typename T, typename... Args>
				T& AddComponent(Args&&... args)
				{
					auto component = std::make_unique<T>(std::forward<Args>(args)...);

					component->m_owner = this;

					T& ref = *component;
					m_components.push_back(std::move(component));

					return ref;
				}

		shared_ptr<Apex::SceneGraph::SceneNode> GetNode() const { return m_owner; }

	private:
		LibMath::Transform				m_localTransform;
		LibMath::Transform				m_globalTransform;

		std::shared_ptr<Apex::SceneGraph::SceneNode>	m_owner;
		std::vector<std::unique_ptr<Apex::Component>>	m_components;
	};
}

#endif // !OBJECT