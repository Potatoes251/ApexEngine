#ifndef PHYSIC
#define PHYSIC

#include <PxPhysicsAPI.h>

#include "LibMath/Vector/Vector3.h"

#include "Rigidbody.h"
#include "Collider.h"

#include <vector>

namespace Apex::Physic
{
	
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

		void Update(float deltaTime_s);
		void AddGround();
		void CreateActor(Collider const& collider, RigidBodyComponent const& rigidBody);

	private:
		void CreateActorDynamic(Collider const& collider, RigidBodyComponent const& rigidBody);
		void CreateActorStatic(Collider const& collider, RigidBodyComponent const& rigidBody);
		void CreateActorKinematic(Collider const& collider, RigidBodyComponent const& rigidBody);

		void SyncToPhysx();
		void SyncFromPhysx();


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

