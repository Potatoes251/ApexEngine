#ifndef OBJECT
#define OBJECT

#include <LibMath/Matrix/Matrix4.h>

namespace Apex::SceneGraph { class SceneNode; }

using std::shared_ptr;
using LibMath::Matrix4;
using Apex::SceneGraph::SceneNode;

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

	private:
		Matrix4					m_transform;
		shared_ptr<SceneNode>	m_owner;
	};
}

#endif // !OBJECT