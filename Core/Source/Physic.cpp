#include "Physic.h"

#include <PxPhysicsAPI.h>
#include <cooking/PxCooking.h>

#include <iostream>
#include <cassert>

#include "Rigidbody.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "MeshCollider.h"
#include "Mesh.h"
#include "Shader.h"

#include "DebugRenderer.h"

using namespace Apex::Physic;
using namespace Apex::Rendering;

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

void PhysicSystem::DrawMeshCollider(physx::PxShape* shape, physx::PxRigidActor* actor, LibMath::Vector3 scale)
{
	if (!shape || !actor)
		return;

	// Compute full world transform: actor * shape local
	physx::PxTransform worldPose = actor->getGlobalPose() * shape->getLocalPose();
	LibMath::Vector3 worldPos(worldPose.p.x, worldPose.p.y, worldPose.p.z);
	LibMath::Quaternion worldRot(worldPose.q.x, worldPose.q.y, worldPose.q.z, worldPose.q.w);

	DebugRenderer& debugRenderer = DebugRenderer::Get();

	physx::PxGeometry const& geom = shape->getGeometry();

	switch (geom.getType())
	{
	case physx::PxGeometryType::eTRIANGLEMESH:
	{
		auto const& triangleGeom = static_cast<physx::PxTriangleMeshGeometry const&>(geom);
		physx::PxTriangleMesh const* mesh = triangleGeom.triangleMesh;
		if (!mesh) return;

		// Get vertices and indices
		const physx::PxVec3* vertices = mesh->getVertices();
		const physx::PxU16* indices = (const physx::PxU16*)(mesh->getTriangles());
		physx::PxU32 numTriangles = mesh->getNbTriangles();

		// Draw each triangle as lines
		for (physx::PxU32 t = 0; t < numTriangles; ++t)
		{
			physx::PxU32 i0 = indices[3 * t + 0];
			physx::PxU32 i1 = indices[3 * t + 1];
			physx::PxU32 i2 = indices[3 * t + 2];

			LibMath::Vector3 v0 = worldPos + (worldRot.rotate(LibMath::Vector3(vertices[i0].x, vertices[i0].y, vertices[i0].z) * scale));
			LibMath::Vector3 v1 = worldPos + (worldRot.rotate(LibMath::Vector3(vertices[i1].x, vertices[i1].y, vertices[i1].z) * scale));
			LibMath::Vector3 v2 = worldPos + (worldRot.rotate(LibMath::Vector3(vertices[i2].x, vertices[i2].y, vertices[i2].z) * scale));

			debugRenderer.AddLine(v0, v1);
			debugRenderer.AddLine(v1, v2);
			debugRenderer.AddLine(v2, v0);
		}
		break;
	}

	case physx::PxGeometryType::eCONVEXMESH:
	{
		auto const& geom = static_cast<physx::PxConvexMeshGeometry const&>(shape->getGeometry());
		physx::PxConvexMesh const* mesh = geom.convexMesh;
		if (!mesh) return;

		const physx::PxU32 numVertices = mesh->getNbVertices();
		const physx::PxVec3* vertices = mesh->getVertices();

		// Convex meshes store polygons; we'll draw edges of each polygon
		for (physx::PxU32 i = 0; i < mesh->getNbPolygons(); ++i)
		{
			physx::PxHullPolygon poly;
			mesh->getPolygonData(i, poly);

			for (physx::PxU32 j = 0; j < poly.mNbVerts; ++j)
			{
				physx::PxU32 v0Idx = mesh->getIndexBuffer()[poly.mIndexBase + j];
				physx::PxU32 v1Idx = mesh->getIndexBuffer()[poly.mIndexBase + ((j + 1) % poly.mNbVerts)];

				LibMath::Vector3 v0 = 
					worldPos + (worldRot.rotate(LibMath::Vector3(vertices[v0Idx].x, vertices[v0Idx].y, vertices[v0Idx].z) * scale));
				LibMath::Vector3 v1 = 
					worldPos + (worldRot.rotate(LibMath::Vector3(vertices[v1Idx].x, vertices[v1Idx].y, vertices[v1Idx].z) * scale));

				debugRenderer.AddLine(v0, v1);
			}
		}
		break;
	}

	default:
		break;
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
	LibMath::Transform trans = rigidBody.GetTransform();
	LibMath::Vector3 pos = trans.getPosition();
	LibMath::Quaternion rot = trans.getRotation();

	physx::PxTransform transform(physx::PxVec3(pos[0], pos[1], pos[2]), physx::PxQuat(rot[0], rot[1], rot[2], rot[3]));

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
			physx::PxTransform trans = actor.m_actor->getGlobalPose();

			actor.m_owner->SetPosition(LibMath::Vector3(trans.p.x, trans.p.y, trans.p.z));
			actor.m_owner->SetRotation(LibMath::Quaternion(trans.q.x, trans.q.y, trans.q.z, trans.q.w));
		}
	}
}

void PhysicSystem::DrawColliders()
{
	for (PhysicActor& actor : m_rigidBodies)
	{
		physx::PxU32 nbShapes = actor.m_actor->getNbShapes();
		std::vector<physx::PxShape*> shapes(nbShapes);
		actor.m_actor->getShapes(shapes.data(), nbShapes);

		LibMath::Matrix4 trans = static_cast<LibMath::Matrix4>(actor.m_owner->GetTransform());

		for (physx::PxShape* shape : shapes)
		{
			physx::PxGeometry const& geometry = shape->getGeometry();

			physx::PxTransform worldPose = actor.m_actor->getGlobalPose() * shape->getLocalPose();
			physx::PxVec3 pos = worldPose.p;
			physx::PxQuat rot = worldPose.q;
			switch (geometry.getType())
			{
			case physx::PxGeometryType::eBOX:
			{
				physx::PxBoxGeometry const& box = static_cast<physx::PxBoxGeometry const&>(geometry);
				physx::PxVec3 halfExtents = box.halfExtents;
				DebugRenderer::Get().AddBox({ pos.x, pos.y, pos.z }, { rot.x, rot.y, rot.z, rot.w }, { halfExtents.x, halfExtents.y, halfExtents.z });
				break;
			}
			case physx::PxGeometryType::eCAPSULE:
			{
				physx::PxCapsuleGeometry const& capsule = static_cast<physx::PxCapsuleGeometry const&>(geometry);
				LibMath::Quaternion fix(LibMath::Radian(0.f), LibMath::Radian(0.f), LibMath::Degree(90));
				DebugRenderer::Get().AddCapsule({ pos.x, pos.y, pos.z }, LibMath::Quaternion{ rot.x, rot.y, rot.z, rot.w } * fix, capsule.radius, capsule.halfHeight);
				break;
			}
			case physx::PxGeometryType::eTRIANGLEMESH:
			case physx::PxGeometryType::eCONVEXMESH:
				DrawMeshCollider(shape, actor.m_actor, actor.m_owner->GetScale());
				break;
			default:
				break;
			}

		}
	}
}

physx::PxTriangleMesh* PhysicSystem::CookTriangleMesh(Apex::Resources::ResourceHandle<Mesh> const& mesh)
{
	physx::PxTriangleMeshDesc meshDesc;

	meshDesc.points.count = static_cast<physx::PxU32>(mesh->m_vertices.size());
	meshDesc.points.stride = sizeof(::Vertex);
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
	meshDesc.points.stride = sizeof(::Vertex);
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