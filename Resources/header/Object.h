#ifndef OBJECT
#define OBJECT

#include <LibMath/Matrix/Matrix4.h>
#include "LibMath/Transform.h"

#include "Component.h"
#include "SceneGraph.h"
#include "../../Physic/Header/Physic.h"

#include <unordered_set>
#include <vector>
#include <memory>
#include <utility>
#include <functional>
#include <set>

#ifdef _WIN32
#undef min
#undef max
#endif

namespace Apex::Data
{
	class Object
	{
	public:
		Object();
		Object(size_t forcedId);
		Object(const LibMath::Transform& localTransform, const LibMath::Transform& globalTransform);
		Object(const Object& other) = delete;
		Object&	operator=(const Object& other) = delete;
		~Object();

		std::unique_ptr<Object>		Clone();

		size_t			GetId() const { return m_id; }
		void			SetId(size_t id);
		void			SetCompId(size_t oldId, size_t newId);
		void			SetCompId(Component* comp, size_t newId);
		size_t			NewId();
		// set id to INVALID_ID
		void			RemoveId();
		void			RemoveCompId(size_t oldId);
		void			RemoveCompId(Component* comp);

		std::string&	GetName() { return m_name; }
		void			SetName(std::string name);

		const	LibMath::Transform&		GetLocalTransform() const;
		const	LibMath::Transform&		GetGlobalTransform();

		void		UpdateGlobalTransform();

		void		SetLocalTransform(const LibMath::Transform& transform);
		void		SetLocalPosition(const LibMath::Vector3& position);
		void		SetLocalRotation(const LibMath::Quaternion& rotation);
		void		SetLocalScale(const LibMath::Vector3& scale);

		const LibMath::Vector3&		GetLocalPosition() const { return m_localTransform.getPosition(); }
		const LibMath::Quaternion&	GetLocalRotation() const { return m_localTransform.getRotation(); }
		const LibMath::Vector3&		GetLocalScale() const { return m_localTransform.getScale(); }

		void		SetGlobalTransform(const LibMath::Transform& transform);

		const LibMath::Vector3&		GetGlobalPosition() const { return m_globalTransform.getPosition(); }
		const LibMath::Quaternion&	GetGlobalRotation() const { return m_globalTransform.getRotation(); }
		const LibMath::Vector3&		GetGlobalScale() const { return m_globalTransform.getScale(); }


		void		Start();
		void		FixedUpdate(float deltaTime_s);
		void		Update(float deltaTime_s);
		void		LateUpdate(float deltaTime_s);
		void		OnTriggerEnter(Physic::Collider* other);
		void		OnTriggerExit(Physic::Collider* other);

		void		AddTag(std::string tag);
		void		RemoveTag(std::string const& tag);
		bool		HasTag(std::string const& tag) const;

		void		Destroy() { m_pendingDestroy = true; }
		bool		IsPendingDestroy() const { return m_pendingDestroy; }

		std::unordered_set<std::string> const& GetTags() const;

		std::shared_ptr<Apex::SceneGraph::SceneNode>	GetSceneNode() { return m_owner; }
		Apex::SceneGraph::SceneNode*					GetSceneNodeRaw() { return m_owner.get(); }

		size_t AddComponentDirect(std::unique_ptr<Apex::Component> comp);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args>(args)...);

			component->m_owner = this;

			T& ref = *component;
			m_components.push_back(std::move(component));

			if (m_freeCompIds.empty())
			{
				m_components.back()->SetId(m_nextCompId++);
			}
			else
			{
				m_components.back()->SetId(*m_freeCompIds.begin());
				m_freeCompIds.erase(m_freeCompIds.begin());
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

		Component* GetComponent(size_t id);

		void RemoveComponent(Component* compToRemove, Apex::Physic::PhysicSystem* phys = nullptr);
		void RemoveComponent(size_t compId, Apex::Physic::PhysicSystem* phys = nullptr);

		std::shared_ptr<Apex::SceneGraph::SceneNode> GetNode() const { return m_owner; }

		void OnChanged();
		void SetOnChangedCallback(std::function<void()> callback);
			
		static constexpr size_t INVALID_ID = std::numeric_limits<size_t>::max();

	private:
		static inline size_t m_nextId = 0;
		static inline std::set<size_t> m_freeIds;

		std::set<size_t> m_freeCompIds;
		size_t m_nextCompId = 0;

		size_t			m_id;
		std::string		m_name;

		LibMath::Transform				m_localTransform;
		LibMath::Transform				m_globalTransform;

		std::function<void()>			m_onChanged;

		// only one of each
		std::unordered_set<std::string>					m_tags;

		std::shared_ptr<Apex::SceneGraph::SceneNode>	m_owner;
		std::vector<std::unique_ptr<Apex::Component>>	m_components;

		bool m_pendingDestroy = false;
	};
}

#endif // !OBJECT