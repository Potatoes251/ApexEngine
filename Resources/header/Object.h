#ifndef OBJECT
#define OBJECT

#include <LibMath/Matrix/Matrix4.h>
#include "LibMath/Transform.h"

#include "Component.h"
#include "SceneGraph.h"
#include "../../Physic/Header/Physic.h"

#include <vector>
#include <memory>
#include <utility>
#include <functional>	

namespace Apex::Data
{
	class Object
	{
	public:
		Object();
		Object(const LibMath::Transform& localTransform, const LibMath::Transform& globalTransform);
		Object(const Object& other) = delete;
		Object&	operator=(const Object& other) = delete;
		~Object() = default;

		std::unique_ptr<Object>		Clone();

		size_t			GetId() const { return m_id; }
		void			SetId(size_t id) { m_id = id; }

		std::string&	GetName() { return m_name; }
		void			SetName(std::string name);

		const	LibMath::Transform&		GetLocalTransform() const;
		const	LibMath::Transform&		GetGlobalTransform();

		void		UpdateGlobalTransform();

		void		SetLocalTransform(const LibMath::Transform& transform);
		void		SetLocalPosition(const LibMath::Vector3& position);
		void		SetLocalRotation(const LibMath::Quaternion& rotation);
		void		SetLocalScale(const LibMath::Vector3& scale);
		void		SetGlobalTransform(const LibMath::Transform& transform);

		void		Start();
		void		FixedUpdate(float deltaTime_s);
		void		Update(float deltaTime_s);
		void		OnTriggerEnter(Physic::Collider* other);
		void		OnTriggerExit(Physic::Collider* other);

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

			if (m_components.size() == 1)
				m_components.back()->SetId(0);
			else
			{
				m_components.back()->SetId(m_components[m_components.size() - 2]->GetId() + 1);
			}

			OnChanged();

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

		template<typename T>
		std::vector<T*> GetComponentsByType()
		{
			std::vector<T*> comps;

			for (auto& comp : m_components)
			{
				T* casted = dynamic_cast<T*>(comp.get());

				if (casted)
					comps.push_back(casted);
			}

			return comps;
		}

		std::vector<Component*> GetComponents() const
		{
			std::vector<Component*> comps;
			for (auto& c : m_components)
				comps.push_back(c.get());
			return comps;
		}

		void RemoveComponent(Component* compToRemove, Apex::Physic::PhysicSystem* phys = nullptr);

		shared_ptr<Apex::SceneGraph::SceneNode> GetNode() const { return m_owner; }

		void OnChanged();
		void SetOnChangedCallback(std::function<void()> callback);
			
	private:
		size_t			m_id;
		static size_t	m_nextId;
		std::string		m_name;

		LibMath::Transform				m_localTransform;
		LibMath::Transform				m_globalTransform;

		std::function<void()>			m_onChanged;

		std::shared_ptr<Apex::SceneGraph::SceneNode>	m_owner;
		std::vector<std::unique_ptr<Apex::Component>>	m_components;
	};
}

#endif // !OBJECT