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
using namespace Apex::Resources;

PhysicSystem::PhysicSystem() : m_simCallback(this)
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
	sceneDesc.simulationEventCallback = &m_simCallback;

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

PhysMatHandle PhysicSystem::CreateMaterial(std::string const& name, float staticFriction, float dynamicFriction, float bounciness)
{
	auto [it, inserted] = m_materialsMap.try_emplace(name, nullptr);
	ResourceEntry* entry = &it->second;

	// If already existing, do not create again
	if (!inserted) return PhysMatHandle(entry);

	entry->m_resource = std::make_unique<PhysicMaterial>(staticFriction, dynamicFriction, bounciness);
	PhysMatHandle handle(entry);

	handle->SetState(ResourceState::Uploaded);
	handle->SetId(m_physxMaterials.size());

	m_physxMaterials.push_back(m_physics->createMaterial(staticFriction, dynamicFriction, bounciness));

	return handle;
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

	if (collider.IsTrigger())
	{
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}

	switch (rigidBody.GetType())
	{
	case BodyType::Dynamic:
		CreateActorDynamic(shape, rigidBody);
		break;
	case BodyType::Static:
		CreateActorStatic(shape, rigidBody);
		break;
	case BodyType::Kinematic:
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

bool PhysicSystem::Raycast(HitResult& hit_out, LibMath::Vector3 position, LibMath::Vector3 direction, float distance)
{
	physx::PxRaycastBuffer hit;
	bool result = m_scene->raycast(ToPxVec(position), ToPxVec(direction.normalize()), distance, hit);

	if (!result) return false;

	hit_out.m_body = static_cast<RigidBodyComponent*>(hit.block.actor->userData);
	hit_out.m_distance = hit.block.distance;
	hit_out.m_position = ToLibVec(hit.block.position);
	hit_out.m_normal = ToLibVec(hit.block.normal);

	return result;
}

bool Apex::Physic::PhysicSystem::Raycast(std::vector<HitResult>& hit_out, LibMath::Vector3 position, LibMath::Vector3 direction, float distance)
{
	hit_out.clear();
	physx::PxRaycastHit hits[16];
	physx::PxRaycastBuffer hitBuffer(hits, 16);

	bool result = m_scene->raycast(ToPxVec(position), ToPxVec(direction.normalize()), distance, hitBuffer);
	if (!result) return false;

	for (physx::PxU32 i = 0; i < hitBuffer.getNbTouches(); ++i)
	{
		const physx::PxRaycastHit& h = hitBuffer.getTouch(i);
		HitResult result;
		result.m_body = static_cast<RigidBodyComponent*>(h.actor->userData);
		result.m_distance = h.distance;
		result.m_position = ToLibVec(h.position);
		result.m_normal = ToLibVec(h.normal);

		hit_out.push_back(result);
	}
	return true;
}

bool PhysicSystem::Sweep(
	HitResult& hit_out, LibMath::Vector3 position, LibMath::Vector3 direction, Collider const& collider, float distance)
{
	physx::PxTransform startPose(ToPxVec(position));

	bool result = false;
	physx::PxSweepBuffer hit;

	switch (collider.GetType())
	{
	case ColliderType::Box:
	{
		BoxCollider const& boxCollider = static_cast<BoxCollider const&>(collider);
		physx::PxBoxGeometry box(ToPxVec(boxCollider.GetHalfExtents()));
		result = m_scene->sweep(box, startPose, ToPxVec(direction.normalize()), distance, hit);
		break;
	}
	case ColliderType::Capsule:
	{
		CapsuleCollider const& capsuleCollider = static_cast<CapsuleCollider const&>(collider);
		physx::PxCapsuleGeometry capsule(capsuleCollider.GetRadius(), capsuleCollider.GetHalfHeight());
		result = m_scene->sweep(capsule, startPose, ToPxVec(direction.normalize()), distance, hit);
		break;
	}
	case ColliderType::Mesh:
		assert(false && "Sweeps with mesh collider are not allowed");
		break;
	default:
		assert(false && "Unhandled collider type");
		break;
	}

	if (!result) return false;
	
	hit_out.m_body = static_cast<RigidBodyComponent*>(hit.block.actor->userData);
	hit_out.m_distance = hit.block.distance;
	hit_out.m_position = LibMath::Vector3(hit.block.position.x, hit.block.position.y, hit.block.position.z);
	hit_out.m_normal = LibMath::Vector3(hit.block.normal.x, hit.block.normal.y, hit.block.normal.z);
	
	return true;
}

bool PhysicSystem::Sweep(
	std::vector<HitResult>& sweep_out, LibMath::Vector3 position, LibMath::Vector3 direction, Collider const& collider, float distance)
{
	sweep_out.clear();
	physx::PxSweepHit sweeps[16];
	physx::PxSweepBuffer sweepBuffer(sweeps, 16);

	physx::PxTransform startPose(ToPxVec(position));

	bool result = false;

	switch (collider.GetType())
	{
	case ColliderType::Box:
	{
		BoxCollider const& boxCollider = static_cast<BoxCollider const&>(collider);
		physx::PxBoxGeometry box(ToPxVec(boxCollider.GetHalfExtents()));
		result = m_scene->sweep(box, startPose, ToPxVec(direction.normalize()), distance, sweepBuffer);
		break;
	}
	case ColliderType::Capsule:
	{
		CapsuleCollider const& capsuleCollider = static_cast<CapsuleCollider const&>(collider);
		physx::PxCapsuleGeometry capsule(capsuleCollider.GetRadius(), capsuleCollider.GetHalfHeight());
		result = m_scene->sweep(capsule, startPose, ToPxVec(direction.normalize()), distance, sweepBuffer);
		break;
	}
	case ColliderType::Mesh:
		assert(false && "Sweeps with mesh collider are not allowed");
		break;
	default:
		assert(false && "Unhandled collider type");
		break;
	}
	if (!result) return false;

	for (physx::PxU32 i = 0; i < sweepBuffer.getNbTouches(); ++i)
	{
		const physx::PxSweepHit& h = sweepBuffer.getTouch(i);
		HitResult result;
		result.m_body = static_cast<RigidBodyComponent*>(h.actor->userData);
		result.m_distance = h.distance;
		result.m_position = ToLibVec(h.position);
		result.m_normal = ToLibVec(h.normal);

		sweep_out.push_back(result);
	}

	return true;
}

void PhysicSystem::Update(float deltaTime_s)
{
	SyncToPhysx();
	m_scene->simulate(deltaTime_s);
	m_scene->fetchResults(true);
	SyncFromPhysx();
}

void PhysicSystem::SyncFromPhysx()
{
	for (PhysicActor& actor : m_rigidBodies)
	{
		if (actor.m_owner->GetType() == BodyType::Dynamic)
		{
			physx::PxTransform trans = actor.m_actor->getGlobalPose();

			actor.m_owner->SetPosition(ToLibVec(trans.p));
			actor.m_owner->SetRotation(ToLibQuat(trans.q));

			if (auto* obj = actor.m_owner->GetOwner())
			{
				obj->GetGlobalTransform().setPosition(ToLibVec(trans.p));
				obj->GetGlobalTransform().setRotation(ToLibQuat(trans.q));
			}
		}
	}
}

void PhysicSystem::DrawMeshCollider(physx::PxShape* shape, physx::PxRigidActor* actor, LibMath::Vector3 scale) const
{
	if (!shape || !actor)
		return;

	physx::PxTransform worldPose = actor->getGlobalPose() * shape->getLocalPose();
	physx::PxGeometry const& geom = shape->getGeometry();

	switch (geom.getType())
	{
	case physx::PxGeometryType::eTRIANGLEMESH:
	{
		physx::PxTriangleMeshGeometry const& triangleGeom = static_cast<physx::PxTriangleMeshGeometry const&>(geom);
		physx::PxTriangleMesh const* mesh = triangleGeom.triangleMesh;
		if (!mesh) return;

		if (mesh->getTriangleMeshFlags() & physx::PxTriangleMeshFlag::e16_BIT_INDICES)
		{
			DrawTriangleMeshCollider16(mesh, worldPose, scale);
		}
		else
		{
			DrawTriangleMeshCollider32(mesh, worldPose, scale);
		}
	}
	break;
	case physx::PxGeometryType::eCONVEXMESH:
	{
		physx::PxConvexMeshGeometry const& geom = static_cast<physx::PxConvexMeshGeometry const&>(shape->getGeometry());
		physx::PxConvexMesh const* mesh = geom.convexMesh;
		if (!mesh) return;

		DrawConvexMeshCollider(mesh, worldPose, scale);
	}
	break;
	default:
		break;
	}
}

void PhysicSystem::DrawTriangleMeshCollider32(
	physx::PxTriangleMesh const* mesh, physx::PxTransform const& worldPose, LibMath::Vector3 const& scale) const
{
	DebugRenderer& debugRenderer = DebugRenderer::Get();

	LibMath::Vector3 worldPos = ToLibVec(worldPose.p);
	LibMath::Quaternion worldRot = ToLibQuat(worldPose.q);

	const physx::PxVec3* vertices = mesh->getVertices();
	const physx::PxU32* indices = (const physx::PxU32*)(mesh->getTriangles());
	physx::PxU32 numTriangles = mesh->getNbTriangles();

	for (physx::PxU32 t = 0; t < numTriangles; ++t)
	{
		physx::PxU32 i0 = indices[3 * t + 0];
		physx::PxU32 i1 = indices[3 * t + 1];
		physx::PxU32 i2 = indices[3 * t + 2];

		LibMath::Vector3 v0 = worldPos + (worldRot.rotate(ToLibVec(vertices[i0]) * scale));
		LibMath::Vector3 v1 = worldPos + (worldRot.rotate(ToLibVec(vertices[i1]) * scale));
		LibMath::Vector3 v2 = worldPos + (worldRot.rotate(ToLibVec(vertices[i2]) * scale));

		debugRenderer.AddLine(v0, v1);
		debugRenderer.AddLine(v1, v2);
		debugRenderer.AddLine(v2, v0);
	}
}

void Apex::Physic::PhysicSystem::DrawTriangleMeshCollider16(
	physx::PxTriangleMesh const* mesh, physx::PxTransform const& worldPose, LibMath::Vector3 const& scale) const
{
	DebugRenderer& debugRenderer = DebugRenderer::Get();

	LibMath::Vector3 worldPos = ToLibVec(worldPose.p);
	LibMath::Quaternion worldRot = ToLibQuat(worldPose.q);

	const physx::PxVec3* vertices = mesh->getVertices();
	const physx::PxU16* indices = (const physx::PxU16*)(mesh->getTriangles());
	physx::PxU32 numTriangles = mesh->getNbTriangles();

	for (physx::PxU32 t = 0; t < numTriangles; ++t)
	{
		physx::PxU32 i0 = indices[3 * t + 0];
		physx::PxU32 i1 = indices[3 * t + 1];
		physx::PxU32 i2 = indices[3 * t + 2];

		LibMath::Vector3 v0 = worldPos + (worldRot.rotate(ToLibVec(vertices[i0]) * scale));
		LibMath::Vector3 v1 = worldPos + (worldRot.rotate(ToLibVec(vertices[i1]) * scale));
		LibMath::Vector3 v2 = worldPos + (worldRot.rotate(ToLibVec(vertices[i2]) * scale));

		debugRenderer.AddLine(v0, v1);
		debugRenderer.AddLine(v1, v2);
		debugRenderer.AddLine(v2, v0);
	}
}

void PhysicSystem::DrawConvexMeshCollider(
	physx::PxConvexMesh const* mesh, physx::PxTransform const& worldPose, LibMath::Vector3 const& scale) const
{
	DebugRenderer& debugRenderer = DebugRenderer::Get();

	LibMath::Vector3 worldPos = ToLibVec(worldPose.p);
	LibMath::Quaternion worldRot = ToLibQuat(worldPose.q);

	const physx::PxU32 numVertices = mesh->getNbVertices();
	const physx::PxVec3* vertices = mesh->getVertices();

	for (physx::PxU32 i = 0; i < mesh->getNbPolygons(); ++i)
	{
		physx::PxHullPolygon poly;
		mesh->getPolygonData(i, poly);

		for (physx::PxU32 j = 0; j < poly.mNbVerts; ++j)
		{
			physx::PxU32 v0Idx = mesh->getIndexBuffer()[poly.mIndexBase + j];
			physx::PxU32 v1Idx = mesh->getIndexBuffer()[poly.mIndexBase + ((j + 1) % poly.mNbVerts)];

			LibMath::Vector3 v0 = worldPos + (worldRot.rotate(ToLibVec(vertices[v0Idx]) * scale));
			LibMath::Vector3 v1 = worldPos + (worldRot.rotate(ToLibVec(vertices[v1Idx]) * scale));

			debugRenderer.AddLine(v0, v1);
		}
	}
}

physx::PxMaterial* Apex::Physic::PhysicSystem::GetMaterial(Collider const& collider) const
{
	physx::PxMaterial* material = m_defaultMaterial;

	PhysMatHandle matHandle = collider.GetMaterial();

	if (matHandle.IsValid())
	{
		size_t id = matHandle->GetId();
		assert(id < m_physxMaterials.size() && "Invalid PhysicMaterial id");

		if (m_physxMaterials[id]) material = m_physxMaterials[id];
	}
	return material;
}

physx::PxShape* PhysicSystem::CreateShapeBox(BoxCollider const& box)
{
	return m_physics->createShape(physx::PxBoxGeometry(ToPxVec(box.GetHalfExtents())), *GetMaterial(box));
}

physx::PxShape* PhysicSystem::CreateShapeCapsule(CapsuleCollider const& capsule)
{
	float halfHeight = capsule.GetHalfHeight();
	float radius = capsule.GetRadius();

	return m_physics->createShape(physx::PxCapsuleGeometry(radius, halfHeight), *GetMaterial(capsule));
}

physx::PxShape* PhysicSystem::CreateShapeMesh(MeshCollider const& mesh, RigidBodyComponent const& rigidBody)
{
	switch (mesh.GetMeshColliderType())
	{
	case MeshColliderType::Triangle:
		if (rigidBody.GetType() != BodyType::Static)
		{
			assert(false && "Triangle mesh can only be static");
			return nullptr;
		}
		{
			LibMath::Vector3 scale = rigidBody.GetScale();
			physx::PxMeshScale meshScale(
				ToPxVec(scale),						// scaling
				physx::PxQuat(physx::PxIdentity)   // rotation
			);
			physx::PxTriangleMeshGeometry geometry(CookTriangleMesh(mesh.GetMesh()), meshScale);
			return m_physics->createShape(geometry, *GetMaterial(mesh));
		}
	case MeshColliderType::Convex:
	{
		LibMath::Vector3 scale = rigidBody.GetScale();
		physx::PxMeshScale meshScale(
			ToPxVec(scale),						// scaling
			physx::PxQuat(physx::PxIdentity)    // rotation
		);

		physx::PxConvexMeshGeometry geometry(CookConvexMesh(mesh.GetMesh()), meshScale);

		return m_physics->createShape(geometry, *GetMaterial(mesh));
	}
	default:
		assert(false && "Unhandled mesh collider type");
		break;
	}
	return nullptr;
}


void PhysicSystem::CreateActorDynamic(physx::PxShape* shape, RigidBodyComponent& rigidBody)
{
	physx::PxTransform transform(ToPxVec(rigidBody.GetPosition()), ToPxQuat(rigidBody.GetRotation()));

	physx::PxRigidDynamic* body = m_physics->createRigidDynamic(transform);

	body->attachShape(*shape);

	physx::PxRigidBodyExt::updateMassAndInertia(*body, rigidBody.GetMass());

	m_scene->addActor(*body);

	body->userData = &rigidBody;

	PhysicActor actor;
	actor.m_actor = body;
	actor.m_owner = &rigidBody;

	m_rigidBodies.push_back(actor);
}

void PhysicSystem::CreateActorStatic(physx::PxShape* shape, RigidBodyComponent& rigidBody)
{
	physx::PxTransform transform(ToPxVec(rigidBody.GetPosition()), ToPxQuat(rigidBody.GetRotation()));

	physx::PxRigidStatic* body = m_physics->createRigidStatic(transform);

	body->attachShape(*shape);

	m_scene->addActor(*body);

	body->userData = &rigidBody;

	PhysicActor actor;
	actor.m_actor = body;
	actor.m_owner = &rigidBody;

	m_rigidBodies.push_back(actor);
}

void PhysicSystem::CreateActorKinematic(physx::PxShape* shape, RigidBodyComponent& rigidBody)
{
	physx::PxTransform transform(ToPxVec(rigidBody.GetPosition()));

	physx::PxRigidDynamic* body = m_physics->createRigidDynamic(transform);

	body->attachShape(*shape);

	body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

	m_scene->addActor(*body);

	body->userData = &rigidBody;

	PhysicActor actor;
	actor.m_actor = body;
	actor.m_owner = &rigidBody;

	m_rigidBodies.push_back(actor);
}

void PhysicSystem::SyncToPhysx()
{
	for (PhysicActor& actor : m_rigidBodies)
	{
		switch (actor.m_owner->GetType())
		{
		case BodyType::Kinematic:
		{
			physx::PxRigidDynamic* body = actor.m_actor->is<physx::PxRigidDynamic>();

			physx::PxTransform target(ToPxVec(actor.m_owner->GetPosition()), ToPxQuat(actor.m_owner->GetRotation()));

			body->setKinematicTarget(target);
			break;
		}
		case BodyType::Dynamic:
		{
			physx::PxRigidDynamic* body = actor.m_actor->is<physx::PxRigidDynamic>();
			if (!body) continue;
			std::vector<Force> forces = actor.m_owner->GetForces();
			for (Force force : forces)
			{
				ApplyForce(body, force);
			}
			actor.m_owner->ResetForces();
			break;
		}
		default:
			break;
		}
	}
}

void PhysicSystem::ApplyForce(physx::PxRigidDynamic* body, Force force)
{
	physx::PxVec3 pxForce = ToPxVec(force.m_force);
	switch (force.m_type)
	{
	case ForceType::Force:
		body->addForce(pxForce, physx::PxForceMode::eFORCE);
		break;
	case ForceType::Impulse:
		body->addForce(pxForce, physx::PxForceMode::eIMPULSE);
		break;
	case ForceType::VelocityChange:
		body->addForce(pxForce, physx::PxForceMode::eVELOCITY_CHANGE);
		break;
	case ForceType::Acceleration:
		body->addForce(pxForce, physx::PxForceMode::eACCELERATION);
		break;
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
			switch (geometry.getType())
			{
			case physx::PxGeometryType::eBOX:
			{
				physx::PxBoxGeometry const& box = static_cast<physx::PxBoxGeometry const&>(geometry);
				DebugRenderer::Get().AddBox(ToLibVec(worldPose.p), ToLibQuat(worldPose.q), ToLibVec(box.halfExtents));
				break;
			}
			case physx::PxGeometryType::eCAPSULE:
			{
				physx::PxCapsuleGeometry const& capsule = static_cast<physx::PxCapsuleGeometry const&>(geometry);
				LibMath::Quaternion fix(LibMath::Radian(0.f), LibMath::Radian(0.f), LibMath::Degree(90));
				DebugRenderer::Get().AddCapsule(ToLibVec(worldPose.p), ToLibQuat(worldPose.q) * fix, capsule.radius, capsule.halfHeight);
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

	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

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

	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

	return m_physics->createConvexMesh(readBuffer);
}



// SimulationCallback

void PhysicSystem::SimulationCallback::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
{
}

void PhysicSystem::SimulationCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
{
}

void PhysicSystem::SimulationCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
}

void PhysicSystem::SimulationCallback::onContact(physx::PxContactPairHeader const& pairHeader, physx::PxContactPair const* pairs, physx::PxU32 nbPairs)
{
}

void PhysicSystem::SimulationCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	for (int i = 0; i < count; i++)
	{
		RigidBodyComponent* otherbody = static_cast<RigidBodyComponent*>(pairs[i].otherActor->userData);
		RigidBodyComponent* body = static_cast<RigidBodyComponent*>(pairs[i].triggerActor->userData);
		switch (pairs[i].status)
		{
		case physx::PxPairFlag::eNOTIFY_TOUCH_FOUND:
			otherbody->OnTriggerEnter(body);
			break;
		case physx::PxPairFlag::eNOTIFY_TOUCH_LOST:
			otherbody->OnTriggerExit(body);
			break;
		default:
			assert(false && "Unhandled Trigger status");
			break;
		}
	}
}

void PhysicSystem::SimulationCallback::onAdvance(physx::PxRigidBody const* const* bodyBuffer, physx::PxTransform const* poseBuffer, physx::PxU32 const count)
{
}
