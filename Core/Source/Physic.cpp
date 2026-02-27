#include "Physic.h"

#include <PxPhysicsAPI.h>

#include <iostream>
#include <cassert>

#include "Rigidbody.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "CaspuleCollider.h"

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
	physx::PxRigidStatic* ground = physx::PxCreatePlane(*m_physics, physx::PxPlane(0, 1, 0, 0), *m_defaultMaterial);

	m_scene->addActor(*ground);
}

void PhysicSystem::CreateActor(Collider const& collider, RigidBodyComponent const& rigidBody)
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


void PhysicSystem::CreateActorDynamic(physx::PxShape* shape, RigidBodyComponent const& rigidBody)
{
	LibMath::Vector3 pos = rigidBody.GetPosition();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]));

	physx::PxRigidDynamic* body = m_physics->createRigidDynamic(transform);

	body->attachShape(*shape);

	physx::PxRigidBodyExt::updateMassAndInertia(*body, rigidBody.GetMass());

	m_scene->addActor(*body);
}

void PhysicSystem::CreateActorStatic(physx::PxShape* shape, RigidBodyComponent const& rigidBody)
{
	LibMath::Vector3 pos = rigidBody.GetPosition();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]));

	physx::PxRigidStatic* body = m_physics->createRigidStatic(transform);

	body->attachShape(*shape);

	m_scene->addActor(*body);
}

void PhysicSystem::CreateActorKinematic(physx::PxShape* shape, RigidBodyComponent const& rigidBody)
{
	LibMath::Vector3 pos = rigidBody.GetPosition();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]));

	physx::PxRigidDynamic* body = m_physics->createRigidDynamic(transform);

	body->attachShape(*shape);

	body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

	m_scene->addActor(*body);
}

void PhysicSystem::SyncToPhysx()
{
	for (PhysicActor& actor : m_rigidBodies)
	{
		if (actor.m_owner->GetType() == BodyType::KINEMATIC)
		{
			physx::PxRigidDynamic* body = actor.m_actor->is<physx::PxRigidDynamic>();
			LibMath::Vector3 pos = actor.m_owner->GetPosition();

			physx::PxTransform target(physx::PxVec3(pos[0], pos[1], pos[2]));

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
		}
	}
}
