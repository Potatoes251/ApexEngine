#include "Physic.h"

#include <PxPhysicsAPI.h>
#include <cooking/PxCooking.h>

#include <iostream>
#include <cassert>

#include "Rigidbody.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "CaspuleCollider.h"
#include "MeshCollider.h"
#include "Mesh.h"

using namespace Apex::Physic;

PhysicSystem::PhysicSystem()
{
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
	if (!m_foundation)
	{
		std::cerr << "PhysX foundation creation failed." << std::endl;
		m_physics = nullptr;
		return;
	}

	physx::PxTolerancesScale scale;

	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, scale, true, nullptr);

	if (!m_physics)
	{
		std::cerr << "PhysX physics creation failed." << std::endl;
		m_foundation->release();
		m_foundation = nullptr;
		return;
	}

	m_dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

	physx::PxSceneDesc sceneDesc(scale);

	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = m_dispatcher;
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

	m_scene = m_physics->createScene(sceneDesc);

	m_defaultMaterial = m_physics->createMaterial(
		0.5f,  // static friction
		0.5f,  // dynamic friction
		0.6f   // restitution (bounciness)
	);
}

PhysicSystem::~PhysicSystem()
{
	if (m_scene)       m_scene->release();
	if (m_dispatcher)  m_dispatcher->release();
	if (m_physics)     m_physics->release();
	if (m_foundation)  m_foundation->release();
}

void PhysicSystem::Update(float deltaTime_s)
{
	SyncToPhysx();
	m_scene->simulate(deltaTime_s);
	m_scene->fetchResults(true);
}

void PhysicSystem::AddGround()
{
	physx::PxRigidStatic* ground = physx::PxCreatePlane(*m_physics, physx::PxPlane(0, 0, 1, 0), *m_defaultMaterial);

	m_scene->addActor(*ground);
}

void PhysicSystem::CreateActor(Collider const& collider, RigidBodyComponent& rigidBody)
{
	physx::PxShape* shape = nullptr;
	switch (collider.GetType())
	{
	case ColliderType::Box:
		shape = CreateShapeBox(dynamic_cast<BoxCollider const&>(collider));
		break;
	case ColliderType::Capsule:
		shape = CreateShapeCapsule(dynamic_cast<CapsuleCollider const&>(collider));
		break;
	case ColliderType::Mesh:
		shape = CreateShapeMesh(dynamic_cast<MeshCollider const&>(collider), rigidBody);
		break;
	default:
		assert(false && "Unhandled collider type");
		return;
	}

	if (!shape) return;

	switch (rigidBody.GetType())
	{
	case BodyType::DYNAMIC:
		CreateActorDynamic(shape, rigidBody);
		break;
	case BodyType::STATIC:
		CreateActorStatic(shape, rigidBody);
		break;
	case BodyType::KINEMATIC:
		CreateActorKinematic(shape, rigidBody);
		break;
	default:
		assert(false && "Unhandled rigidbody type");
		return;
	}

	shape->release();
}

void PhysicSystem::RemoveActor(RigidBodyComponent const& rigidbody)
{
	for (auto it = m_rigidBodies.begin(); it != m_rigidBodies.end(); ++it)
	{
		if (it->m_owner == &rigidbody)
		{
			m_scene->removeActor(*(it->m_actor));
			it->m_actor->release();
			m_rigidBodies.erase(it);
			return;
		}
	}
}

physx::PxShape* PhysicSystem::CreateShapeBox(BoxCollider const& box)
{
	LibMath::Vector3 halfExtents = box.GetHalfExtents();

	return m_physics->createShape(
		physx::PxBoxGeometry(physx::PxVec3(halfExtents[0], halfExtents[1], halfExtents[2])), *m_defaultMaterial);
}

physx::PxShape* PhysicSystem::CreateShapeCapsule(CapsuleCollider const& capsule)
{
	float halfHeight = capsule.GetHalfHeight();
	float radius = capsule.GetRadius();

	return m_physics->createShape(physx::PxCapsuleGeometry(radius, halfHeight), *m_defaultMaterial);
}

physx::PxShape* PhysicSystem::CreateShapeMesh(MeshCollider const& mesh, RigidBodyComponent const& rigidBody)
{
	switch (mesh.GetMeshColliderType())
	{
	case MeshColliderType::Triangle:
		if (rigidBody.GetType() != BodyType::STATIC)
		{
			assert(false && "Triangle mesh can only be static");
			return nullptr;
		}
		{
			LibMath::Vector3 scale = rigidBody.GetScale();
			physx::PxMeshScale meshScale(
				{ scale[0], scale[1], scale[2] },  // scaling
				physx::PxQuat(physx::PxIdentity)           // rotation (usually identity)
			);
			physx::PxTriangleMeshGeometry geometry(CookTriangleMesh(mesh.GetMesh()), meshScale);
			return m_physics->createShape(geometry, *m_defaultMaterial);
		}
	case MeshColliderType::Convex:
	{
		LibMath::Vector3 scale = rigidBody.GetScale();
		physx::PxMeshScale meshScale(
			{ scale[0], scale[1], scale[2] },  // scaling
			physx::PxQuat(physx::PxIdentity)           // rotation (usually identity)
		);

		physx::PxConvexMeshGeometry geometry(CookConvexMesh(mesh.GetMesh()), meshScale);

		return m_physics->createShape(geometry, *m_defaultMaterial);
		break;
	}
	default:
		assert(false && "Unhandled mesh collider type");
		break;
	}
	return nullptr;
}


void PhysicSystem::CreateActorDynamic(physx::PxShape* shape, RigidBodyComponent& rigidBody)
{
	LibMath::Transform trans = rigidBody.GetTransform();
	LibMath::Vector3 pos = trans.getPosition();
	LibMath::Quaternion rot = trans.getRotation();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]), physx::PxQuat(rot[0], rot[1], rot[2], rot[3]));

	physx::PxRigidDynamic* body = m_physics->createRigidDynamic(transform);

	body->attachShape(*shape);

	physx::PxRigidBodyExt::updateMassAndInertia(*body, rigidBody.GetMass());

	m_scene->addActor(*body);

	PhysicActor actor;
	actor.m_actor = body;
	actor.m_owner = &rigidBody;

	m_rigidBodies.push_back(actor);
}

void PhysicSystem::CreateActorStatic(physx::PxShape* shape, RigidBodyComponent& rigidBody)
{
	LibMath::Vector3 pos = rigidBody.GetPosition();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]));

	physx::PxRigidStatic* body = m_physics->createRigidStatic(transform);

	body->attachShape(*shape);

	m_scene->addActor(*body);

	PhysicActor actor;
	actor.m_actor = body;
	actor.m_owner = &rigidBody;

	m_rigidBodies.push_back(actor);
}

void PhysicSystem::CreateActorKinematic(physx::PxShape* shape, RigidBodyComponent& rigidBody)
{
	LibMath::Vector3 pos = rigidBody.GetPosition();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]));

	physx::PxRigidDynamic* body = m_physics->createRigidDynamic(transform);

	body->attachShape(*shape);

	body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

	m_scene->addActor(*body);

	PhysicActor actor;
	actor.m_actor = body;
	actor.m_owner = &rigidBody;

	m_rigidBodies.push_back(actor);
}

void PhysicSystem::SyncToPhysx()
{
	for (PhysicActor& actor : m_rigidBodies)
	{
		if (actor.m_owner->GetType() == BodyType::KINEMATIC)
		{
			physx::PxRigidDynamic* body = actor.m_actor->is<physx::PxRigidDynamic>();
			LibMath::Vector3 pos = actor.m_owner->GetPosition();
			LibMath::Quaternion rot = actor.m_owner->GetRotation();

			physx::PxTransform target(physx::PxVec3(pos[0], pos[1], pos[2]), physx::PxQuat(rot[0], rot[1], rot[2], rot[3]));

			body->setKinematicTarget(target);
		}
	}
}

void PhysicSystem::SyncFromPhysx()
{
	for (PhysicActor& actor : m_rigidBodies)
	{
		if (actor.m_owner->GetType() == BodyType::DYNAMIC)
		{
			physx::PxTransform transform = actor.m_actor->getGlobalPose();

			actor.m_owner->SetPosition(LibMath::Vector3(transform.p.x, transform.p.y, transform.p.z));
			actor.m_owner->SetRotation(LibMath::Quaternion(transform.q.x, transform.q.y, transform.q.z, transform.q.w));
		}
	}
}

physx::PxTriangleMesh* PhysicSystem::CookTriangleMesh(Apex::Resources::ResourceHandle<Mesh> const& mesh)
{
	physx::PxTriangleMeshDesc meshDesc;

	meshDesc.points.count = static_cast<physx::PxU32>(mesh->m_vertices.size());
	meshDesc.points.stride = sizeof(Vertex);
	meshDesc.points.data = mesh->m_vertices.data();

	meshDesc.triangles.count = static_cast<physx::PxU32>(mesh->m_indices.size() / 3);
	meshDesc.triangles.stride = sizeof(int) * 3;
	meshDesc.triangles.data = mesh->m_indices.data();

	physx::PxTolerancesScale scale;
	physx::PxCookingParams params(scale);

	physx::PxDefaultMemoryOutputStream writeBuffer;

	if (!PxCookTriangleMesh(params, meshDesc, writeBuffer))
	{
		assert(false && "Triangle mesh cooking failed");
		return nullptr;
	}

	physx::PxDefaultMemoryInputData readBuffer(
		writeBuffer.getData(),
		writeBuffer.getSize()
	);

	return m_physics->createTriangleMesh(readBuffer);
}

physx::PxConvexMesh* PhysicSystem::CookConvexMesh(Apex::Resources::ResourceHandle<Mesh> const& mesh)
{
	physx::PxConvexMeshDesc meshDesc;

	meshDesc.points.count = static_cast<physx::PxU32>(mesh->m_vertices.size());
	meshDesc.points.stride = sizeof(Vertex);
	meshDesc.points.data = mesh->m_vertices.data();

	meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

	physx::PxTolerancesScale scale;
	physx::PxCookingParams params(scale);

	physx::PxDefaultMemoryOutputStream writeBuffer;

	if (!PxCookConvexMesh(params, meshDesc, writeBuffer))
	{
		assert(false && "Convex mesh cooking failed");
		return nullptr;
	}

	physx::PxDefaultMemoryInputData readBuffer(
		writeBuffer.getData(),
		writeBuffer.getSize()
	);

	return m_physics->createConvexMesh(readBuffer);
}