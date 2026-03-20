#ifndef OBJECT
#define OBJECT

#include <LibMath/Matrix/Matrix4.h>
#include "LibMath/Transform.h"

#include "Component.h"
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
		Object&	operator=(const Object& other) = delete;
		~Object() = default;

		std::unique_ptr<Object>		Clone();

		size_t			GetId() const { return m_id; }
		void			SetId(size_t id) { m_id = id; }

		std::string&	GetName() { return m_name; }
		void			SetName(std::string name) { m_name = name; }


				LibMath::Transform&		GetLocalTransform();
		const	LibMath::Transform&		GetLocalTransform() const;
		const	LibMath::Transform&		GetGlobalTransform();

		void		UpdateGlobalTransform();

		void		SetLocalTransform(const LibMath::Transform& transform);
		void		SetGlobalTransform(const LibMath::Transform& transform);

		void		Start();
		void		Update(float deltaTime);

		std::shared_ptr<Apex::SceneGraph::SceneNode>	GetSceneNode() { return m_owner; }
		Apex::SceneGraph::SceneNode*					GetSceneNodeRaw() { return m_owner.get(); }

		void AddComponentDirect(std::unique_ptr<Apex::Component> comp);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args>(args)...);

			component->m_owner = this;

			T& ref = *component;
			m_components.push_back(std::move(component));

			return ref;
		}

		template<typename T>
		T* GetComponent()
		{
			for (auto& comp : m_components)
			{
				T* casted = dynamic_cast<T*>(comp.get());

				if (casted)
					return casted;
			}

			return nullptr;
		}

		std::vector<Component*> GetComponents() const
		{
			std::vector<Component*> comps;
			for (auto& c : m_components)
				comps.push_back(c.get());
			return comps;
		}

		shared_ptr<Apex::SceneGraph::SceneNode> GetNode() const { return m_owner; }

	private:
		size_t			m_id;
		static size_t	m_nextId;
		std::string		m_name;

		LibMath::Transform				m_localTransform;
		LibMath::Transform				m_globalTransform;

		std::shared_ptr<Apex::SceneGraph::SceneNode>	m_owner;
		std::vector<std::unique_ptr<Apex::Component>>	m_components;
	};
}

#endif // !OBJECT