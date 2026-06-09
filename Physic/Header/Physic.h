#ifndef PHYSIC
#define PHYSIC

#include <PxPhysicsAPI.h>

#include "LibMath/Vector/Vector3.h"
#include "LibMath/Quaternion.h"

#include "Mesh.h"
#include "Model.h"
#include "ResourceHandle.h"

#include "Collider.h"

#include <vector>
#include <unordered_map>

namespace Apex::Physic
{
	inline LibMath::Vector3 GRAVITY = { 0.f, -9.81f, 0.f };

	class BoxCollider;
	class MeshCollider;
	class CapsuleCollider;
	class RigidBodyComponent;
	struct Force;

	struct PhysicActor
	{
		physx::PxRigidActor* m_actor;
		RigidBodyComponent* m_owner;
	};

	struct HitResult
	{
		size_t				m_objId;
		size_t				m_colliderId;
		size_t				m_bodyId;
		LibMath::Vector3	m_position;
		LibMath::Vector3	m_normal;
		float				m_distance;
	};

	class PhysicSystem
	{
	public:
		PhysicSystem();
		PhysicSystem(PhysicSystem const&) = delete;
		PhysicSystem& operator=(PhysicSystem const&) = delete;

		~PhysicSystem();

		void			CreateActor(Collider const& collider, RigidBodyComponent& rigidBody);
		void			RemoveActor(RigidBodyComponent const& rigidbody);

		bool			Raycast(HitResult& hit_out, LibMath::Vector3 position, LibMath::Vector3 direction, float distance, bool onlyStatic = false);
		bool			Raycast(std::vector<HitResult>& hit_out, LibMath::Vector3 position, LibMath::Vector3 direction, float distance);
		bool			Sweep(HitResult& hit_out, LibMath::Vector3 position, LibMath::Vector3 direction, Collider const& collider, float distance);
		bool			Sweep(std::vector<HitResult>& hit_out, LibMath::Vector3 position, LibMath::Vector3 direction, Collider const& collider, float distance);

		// should be called inside a fixed update
		void	Update(float deltaTime_s);

		void	DrawColliders();
		void	EditorSync();

	private:
		// class to get the events callback for physic interaction
		class SimulationCallback : public physx::PxSimulationEventCallback
		{
		public:
			SimulationCallback(PhysicSystem* system) : m_system(system) {}
			~SimulationCallback() = default;

			void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {}
			void onWake(physx::PxActor** actors, physx::PxU32 count) override {}
			void onSleep(physx::PxActor** actors, physx::PxU32 count) override {}
			void onContact(physx::PxContactPairHeader const& pairHeader, physx::PxContactPair const* pairs, physx::PxU32 nbPairs) override {}
			void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;

			void onAdvance(physx::PxRigidBody const* const* bodyBuffer, physx::PxTransform const* poseBuffer, physx::PxU32 const count) override {}

		private:
			PhysicSystem* m_system; // back-pointer to the wrapper
		};

		struct PhysXContext
		{
			physx::PxDefaultAllocator        m_allocator;
			physx::PxDefaultErrorCallback    m_errorCallback;

			physx::PxFoundation* m_foundation = nullptr;
			physx::PxPhysics* m_physics = nullptr;

			physx::PxDefaultCpuDispatcher* m_dispatcher = nullptr;
		};

		void	DrawMeshCollider(physx::PxShape* shape, physx::PxRigidActor* actor, LibMath::Vector3 scale) const;
		void	DrawTriangleMeshCollider32(
			physx::PxTriangleMesh const* mesh, physx::PxTransform const& worldPose, LibMath::Vector3 const& scale) const;

		void	DrawTriangleMeshCollider16(
			physx::PxTriangleMesh const* mesh, physx::PxTransform const& worldPose, LibMath::Vector3 const& scale) const;
		
		void	DrawConvexMeshCollider(
			physx::PxConvexMesh const* mesh, physx::PxTransform const& worldPose, LibMath::Vector3 const& scale) const;


		physx::PxMaterial*	GetMaterial(Collider const& collider);

		physx::PxShape*		CreateShapeBox(BoxCollider const& box);
		physx::PxShape*		CreateShapeCapsule(CapsuleCollider const& capsule);
		physx::PxShape*		CreateShapeMesh(MeshCollider const& mesh, RigidBodyComponent const& rigidBody);

		void	CreateActorDynamic(physx::PxShape* shape, RigidBodyComponent& rigidBody);
		void	CreateActorStatic(physx::PxShape* shape, RigidBodyComponent& rigidBody);
		void	CreateActorKinematic(physx::PxShape* shape, RigidBodyComponent& rigidBody);

		void	SyncToPhysx();
		void	SyncFromPhysx();
		void	ApplyForce(physx::PxRigidDynamic* body, Force force);

		physx::PxTriangleMesh*	CookTriangleMesh(Resources::ResourceHandle<Model> const& model);
		physx::PxConvexMesh*	CookConvexMesh(Resources::ResourceHandle<Model> const& model);

		static physx::PxVec3			ToPxVec(LibMath::Vector3 vec) { return { vec[0], vec[1], vec[2] }; }
		static LibMath::Vector3			ToLibVec(physx::PxVec3 vec) { return { vec.x, vec.y, vec.z }; }
		static LibMath::Quaternion		ToLibQuat(physx::PxQuat quat) { return { quat.x, quat.y, quat.z, quat.w }; }
		static physx::PxQuat			ToPxQuat(LibMath::Quaternion quat) { return { quat[0], quat[1], quat[2], quat[3] }; }

		static inline PhysXContext		m_pxCtx;
		static inline bool				m_isCtxInit = false;
		static inline int				m_physicInstanceCount = 0;

		std::vector<physx::PxMaterial*>	m_physxMaterials;
		std::vector<PhysicActor>		m_rigidBodies;

		SimulationCallback				m_simCallback;

		physx::PxScene*					m_scene	= nullptr;

		physx::PxMaterial*				m_defaultMaterial = nullptr;
	};

}

#endif // !PHYSIC