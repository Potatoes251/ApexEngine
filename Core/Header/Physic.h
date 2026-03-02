#ifndef PHYSIC
#define PHYSIC

#include <PxPhysicsAPI.h>

#include "LibMath/Vector/Vector3.h"

#include "ResourceHandle.h"
#include "Mesh.h"

#include <vector>

namespace Apex::Physic
{
	class Collider;
	class BoxCollider;
	class MeshCollider;
	class CapsuleCollider;
	class RigidBodyComponent;


	struct PhysicActor
	{
		physx::PxRigidActor* m_actor;
		RigidBodyComponent* m_owner;
	};

	class PhysicSystem
	{
	public:
		PhysicSystem();
		PhysicSystem(PhysicSystem const&) = delete;
		PhysicSystem& operator=(PhysicSystem const&) = delete;

		~PhysicSystem();

		// should be called inside a fixed update
		void	Update(float deltaTime_s);

		void	AddGround();
		void	CreateActor(Collider const& collider, RigidBodyComponent& rigidBody);
		void	RemoveActor(RigidBodyComponent const& rigidbody);

		// called on frame update
		void	SyncFromPhysx();
		physx::PxTriangleMesh* CookTriangleMesh(Apex::Resources::ResourceHandle<Mesh> const& mesh);
		physx::PxConvexMesh* CookConvexMesh(Apex::Resources::ResourceHandle<Mesh> const& mesh);

	private:
		physx::PxShape*		CreateShapeBox(BoxCollider const& box);
		physx::PxShape*		CreateShapeCapsule(CapsuleCollider const& capsule);
		physx::PxShape*		CreateShapeMesh(MeshCollider const& mesh, RigidBodyComponent const& rigidBody);

		void	CreateActorDynamic(physx::PxShape* shape, RigidBodyComponent& rigidBody);
		void	CreateActorStatic(physx::PxShape* shape, RigidBodyComponent& rigidBody);
		void	CreateActorKinematic(physx::PxShape* shape, RigidBodyComponent& rigidBody);

		void	SyncToPhysx();

		std::vector<PhysicActor>		m_rigidBodies;

		physx::PxDefaultAllocator		m_allocator;
		physx::PxDefaultErrorCallback	m_errorCallback;
		physx::PxDefaultCpuDispatcher*	m_dispatcher = nullptr;
		physx::PxFoundation*			m_foundation = nullptr;
		physx::PxPhysics*				m_physics = nullptr;
		physx::PxScene*					m_scene	= nullptr;
		physx::PxPvd*					m_pvd = nullptr;

		physx::PxMaterial*				m_defaultMaterial = nullptr;
	};

}




#endif // !PHYSIC

