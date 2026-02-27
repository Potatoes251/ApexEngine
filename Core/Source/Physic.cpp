#include "Physic.h"

#include <PxPhysicsAPI.h>
#include <iostream>

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
	switch (rigidBody.GetType())
	{
	case BodyType::DYNAMIC:
		CreateActorDynamic(collider, rigidBody);
		break;
	case BodyType::STATIC:
		CreateActorStatic(collider, rigidBody);
		break;
	}
}

void PhysicSystem::CreateActorDynamic(Collider const& collider, RigidBodyComponent const& rigidBody)
{
	LibMath::Vector3 pos = rigidBody.GetPosition();
	LibMath::Vector3 halfExtents = collider.GetHalfExtents();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]));

	physx::PxRigidDynamic* body = m_physics->createRigidDynamic(transform);

	physx::PxShape* shape = m_physics->createShape(
		physx::PxBoxGeometry(physx::PxVec3(halfExtents[0], halfExtents[1], halfExtents[2])),
		*m_defaultMaterial);

	body->attachShape(*shape);

	physx::PxRigidBodyExt::updateMassAndInertia(*body, rigidBody.GetMass());

	m_scene->addActor(*body);

	shape->release();
}

void PhysicSystem::CreateActorStatic(Collider const& collider, RigidBodyComponent const& rigidBody)
{
	LibMath::Vector3 pos = rigidBody.GetPosition();
	LibMath::Vector3 halfExtents = collider.GetHalfExtents();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]));

	physx::PxRigidStatic* body = m_physics->createRigidStatic(transform);

	physx::PxShape* shape = m_physics->createShape(
		physx::PxBoxGeometry(physx::PxVec3(halfExtents[0], halfExtents[1], halfExtents[2])),
		*m_defaultMaterial);

	body->attachShape(*shape);

	m_scene->addActor(*body);

	shape->release();
}

void PhysicSystem::CreateActorKinematic(Collider const& collider, RigidBodyComponent const& rigidBody)
{
	LibMath::Vector3 pos = rigidBody.GetPosition();
	LibMath::Vector3 halfExtents = collider.GetHalfExtents();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]));

	physx::PxRigidDynamic* body = m_physics->createRigidDynamic(transform);

	physx::PxShape* shape = m_physics->createShape(
		physx::PxBoxGeometry(physx::PxVec3(halfExtents[0], halfExtents[1], halfExtents[2])),
		*m_defaultMaterial);

	body->attachShape(*shape);

	body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

	m_scene->addActor(*body);

	shape->release();
}
