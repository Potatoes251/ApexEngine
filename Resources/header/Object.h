#ifndef OBJECT
#define OBJECT

#include <LibMath/Matrix/Matrix4.h>
#include "../../LowRenderer/header/SceneGraph.h"

using LibMath::Matrix4;
using Apex::SceneGraph::SceneNode;

namespace Apex::Data
{
	class Object
	{
	public:
		Object() = default;
		~Object() = default;

		Matrix4	GetTransform();
	private:
		SceneNode m_sceneNode;
	};
}

#endif // !OBJECT
