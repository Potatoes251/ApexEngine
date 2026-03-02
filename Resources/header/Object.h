#ifndef OBJECT
#define OBJECT

#include <LibMath/Matrix/Matrix4.h>

#include "../../Core/Header/Component.h"

#include <vector>

namespace Apex::SceneGraph { class SceneNode; }

using std::shared_ptr;
using LibMath::Matrix4;
using Apex::SceneGraph::SceneNode;
using Apex::Component;

namespace Apex::Data
{
	class Object
	{
	public:
		Object();
		Object(const Matrix4& transform);
		Object(const Object& other);
		~Object() = default;

				Object&		operator=(const Object& other) = delete;
				Matrix4		GetTransform();
		const	Matrix4		GetTransform() const;
				void		SetTransform(const Matrix4& transform);

				void		Start();
				void		Update(float deltaTime);

				template<typename T, typename... Args>
				T& AddComponent(Args&&... args)
				{
					auto component = std::make_unique<T>(std::forward<Args>(args)...);

					component->m_owner = this;

					T& ref = *component;
					m_components.push_back(std::move(component));

					return ref;
				}

	private:
		Matrix4					m_transform;
		shared_ptr<SceneNode>	m_owner;

		std::vector<std::unique_ptr<Component>>		m_components;
	};
}

#endif // !OBJECT