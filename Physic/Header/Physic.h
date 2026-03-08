#ifndef PHYSIC
#define PHYSIC

#include <PxPhysicsAPI.h>

#include "LibMath/Vector/Vector3.h"

#include "ResourceHandle.h"
#include "Mesh.h"
#include "Shader.h"

#include "PhysicMaterial.h"
#include "Collider.h"

#include <vector>
#include <unordered_map>

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

		PhysMatHandle	CreateMaterial(std::string const& name, float staticFriction, float dynamicFriction, float bounciness);
		void			CreateActor(Collider const& collider, RigidBodyComponent& rigidBody);
		void			RemoveActor(RigidBodyComponent const& rigidbody);

		// should be called inside a fixed update
		void	Update(float deltaTime_s);

		// called on frame update
		void	SyncFromPhysx();

		void	DrawColliders();

	private:
		void	DrawMeshCollider(physx::PxShape* shape, physx::PxRigidActor* actor, LibMath::Vector3 scale) const;
		void	DrawTriangleMeshCollider32(
			physx::PxTriangleMesh const* mesh, physx::PxTransform const& worldPose, LibMath::Vector3 const& scale) const;

		void	DrawTriangleMeshCollider16(
			physx::PxTriangleMesh const* mesh, physx::PxTransform const& worldPose, LibMath::Vector3 const& scale) const;
		
		void	DrawConvexMeshCollider(
			physx::PxConvexMesh const* mesh, physx::PxTransform const& worldPose, LibMath::Vector3 const& scale) const;


		physx::PxMaterial*	GetMaterial(Collider const& collider) const;

		physx::PxShape*		CreateShapeBox(BoxCollider const& box);
		physx::PxShape*		CreateShapeCapsule(CapsuleCollider const& capsule);
		physx::PxShape*		CreateShapeMesh(MeshCollider const& mesh, RigidBodyComponent const& rigidBody);

		void	CreateActorDynamic(physx::PxShape* shape, RigidBodyComponent& rigidBody);
		void	CreateActorStatic(physx::PxShape* shape, RigidBodyComponent& rigidBody);
		void	CreateActorKinematic(physx::PxShape* shape, RigidBodyComponent& rigidBody);

		void	SyncToPhysx();

		physx::PxTriangleMesh*	CookTriangleMesh(Apex::Resources::ResourceHandle<Mesh> const& mesh);
		physx::PxConvexMesh*	CookConvexMesh(Apex::Resources::ResourceHandle<Mesh> const& mesh);

		std::vector<PhysicActor>		m_rigidBodies;
		std::vector<physx::PxMaterial*>	m_physxMaterials;

		std::unordered_map<std::string, Apex::Resources::ResourceEntry>			
										m_materialsMap;

		physx::PxDefaultAllocator		m_allocator;
		physx::PxDefaultErrorCallback	m_errorCallback;
		physx::PxDefaultCpuDispatcher*	m_dispatcher = nullptr;
		physx::PxFoundation*			m_foundation = nullptr;
		physx::PxPhysics*				m_physics = nullptr;
		physx::PxScene*					m_scene	= nullptr;

		physx::PxMaterial*				m_defaultMaterial = nullptr;
	};

}




#endif // !PHYSIC

