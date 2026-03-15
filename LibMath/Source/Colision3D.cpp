#include "LibMath/Colision3D.h"


bool	LibMath::collisionLinePlane(Line3D line, Plane plane)
{
	return line.getDirection().dot(plane.getNormal()) != 0;
}

bool	LibMath::collisionTwoPlane(Plane plane1, Plane plane2)
{
	if (plane1.getNormal() != plane2.getNormal() &&
		plane1.getNormal() != -plane2.getNormal())
		return true;

	return fabs(plane1.getDistance() - plane2.getDistance()) < g_epsilon;
}


bool	LibMath::collisionPointSphere(Point3D point, Sphere sphere)
{
	return sphere.getCenter().distanceSquare(point) <= sphere.getRadius() * sphere.getRadius();
}

bool	LibMath::collisionRaySphere(Line3D line3D, Sphere sphere)
{
	Vector3 m = line3D.getCenter() - sphere.getCenter();

	float b = m.dot(line3D.getDirection());
	float c = m.dot(m) - sphere.getRadius() * sphere.getRadius();

	float delta = b * b - c;

	if (delta < 0.f)
	{
		return false;
	}

	float deltaRoot = LibMath::squareRoot(delta);

	float t1 = -b - deltaRoot;
	float t2 = -b + deltaRoot;

	return 0 < t1 || 0 < t2;
}

bool	LibMath::collisionPlaneSphere(Plane plane, Sphere sphere)
{
	return plane.getPointOnPlane().distance(sphere.getCenter()) < sphere.getRadius();
}

bool	LibMath::collisionTwoSphere(Sphere sphere1, Sphere sphere2)
{
	float radiusSquared = (sphere1.getRadius() + sphere2.getRadius()) * (sphere1.getRadius() + sphere2.getRadius());

	return sphere1.getCenter().distanceSquare(sphere2.getCenter()) < radiusSquared;
}

bool	LibMath::collisionPointCapsule(Point3D point, Capsule capsule)
{
	LibMath::Vector3 capsuleSeg = capsule.getEnd() - capsule.getStart();
	LibMath::Vector3 AP = point - capsule.getStart();

	float t = capsuleSeg.dot(AP) / capsuleSeg.magnitudeSquared();

	t = LibMath::clamp(t, 0.f, 1.f);

	float distance = (point - (capsule.getStart() + capsuleSeg * t)).magnitudeSquared();

	return distance < capsule.getRadius() * capsule.getRadius();
}

bool	LibMath::collisionRayCapsule(Line3D ray, Capsule capsule)
{
	LibMath::Vector3 capsuleSegment = capsule.getEnd() - capsule.getStart();
	LibMath::Vector3 AP = ray.getCenter() - capsule.getStart();

	float u = capsuleSegment.dot(AP) / capsuleSegment.magnitudeSquared();
	u = LibMath::clamp(u, 0.f, 1.f);

	LibMath::Vector3 capPoint = capsule.getStart() + capsuleSegment * u;

	float t = (capsule.getStart() + (-ray.getCenter())).dot(ray.getDirection());
	t = t < 0 ? 0 : t;

	LibMath::Vector3 rayPoint = ray.getCenter() + ray.getDirection() * t;

	float distance = capPoint.distanceSquaredFrom(rayPoint);

	if (distance < capsule.getRadius() * capsule.getRadius())
	{
		return true;
	}

	return collisionRaySphere(ray, Sphere(capsule.getStart(), capsule.getRadius())) ||
			collisionRaySphere(ray, Sphere(capsule.getEnd(), capsule.getRadius()));
}

bool	LibMath::collisionPlaneCapsule(Plane plane, Capsule capsule)
{
	Point3D planePoint = plane.getPointOnPlane();
	Vector3 normal = plane.getNormal();
	normal.normalize();

	float da = (capsule.getStart() - planePoint).dot(plane.getNormal());
	float db = (capsule.getEnd() - planePoint).dot(plane.getNormal());

	return std::min(da, db) <= capsule.getRadius() && std::max(da, db) >= -capsule.getRadius();
}

bool	LibMath::collisionSphereCapsule(Sphere sphere, Capsule capsule)
{
	LibMath::Vector3 AB = capsule.getEnd() - capsule.getStart();
	LibMath::Vector3 AP = sphere.getCenter() - capsule.getStart();

	float t = AB.dot(AP) / AB.magnitudeSquared();

	t = LibMath::clamp(t, 0.f, 1.f);

	float distance = (sphere.getCenter() - (capsule.getStart() + AB * t)).magnitudeSquared();
	float radiusSum = capsule.getRadius() + sphere.getRadius();

	return distance < radiusSum * radiusSum;
}

bool	LibMath::collisionTwoCapsule(Capsule cap1, Capsule cap2)
{
	Vector3 u = cap1.getEnd() - cap1.getStart();
	Vector3 v = cap2.getEnd() - cap2.getStart();
	Vector3 w = cap2.getStart() - cap1.getStart();
	
	float a = u.dot(u);
	float b = u.dot(v);
	float c = v.dot(v);
	float d = u.dot(w);
	float e = v.dot(w);

	float s = 0;
	float t = 0;

	float denom = (a * c - b * b);

	if (denom < g_epsilon)
	{
		s = u.dot(w) / a;
		s = clamp(s, 0.0f, 1.0f);

		t = e / c;
		t = clamp(t, 0.0f, 1.0f);
	}
	else
	{
		s = (b * e - c * d) / denom;
		t = (a * e - b * d) / denom;

		s = clamp(s, 0.0f, 1.0f);
		t = clamp(t, 0.0f, 1.0f);
	}

	Vector3 C1 = cap1.getStart() + u * s;
	Vector3 C2 = cap2.getStart() + v * t;

	float radiusSquared = (cap1.getRadius() + cap2.getRadius()) * (cap1.getRadius() + cap2.getRadius());

	return C1.distanceSquaredFrom(C2) <= radiusSquared;
}


bool	LibMath::collisionPointPrism(Point3D point, Prism prism)
{
	return (point.getX() <= prism.getCenter().getX() + prism.extentX() &&
			point.getY() <= prism.getCenter().getY() + prism.extentY() &&
			point.getZ() <= prism.getCenter().getZ() + prism.extentZ() &&
			point.getX() >= prism.getCenter().getX() &&
			point.getY() >= prism.getCenter().getY() &&
			point.getZ() >= prism.getCenter().getZ());
}

bool	LibMath::collisionRayPrism(Line3D ray, Prism prism)
{
	Point3D halfSize(prism.extentX(), prism.extentY(), prism.extentZ());

	LibMath::Vector3 minPoint = prism.getCenter() - halfSize;
	LibMath::Vector3 maxPoint = prism.getCenter() + halfSize;

	LibMath::Vector3 rayCenter(ray.getCenter().getX(), ray.getCenter().getY(), ray.getCenter().getZ());

	float tMin = -std::numeric_limits<float>::infinity();
	float tMax = std::numeric_limits<float>::infinity();

	for (int i = 0; i < 3; ++i)
	{
		float origin = rayCenter[i];
		float dir = ray.getDirection()[i];
		float minBound = minPoint[i];
		float maxBound = maxPoint[i];

		if (std::abs(dir) < g_epsilon)
		{
			// Ray is parallel to slab. No hit if origin not within slab
			if (origin < minBound || origin > maxBound)
			{
				return false;
			}
		}
		else
		{
			float invD = 1.0f / dir;
			float t1 = (minBound - origin) * invD;
			float t2 = (maxBound - origin) * invD;

			if (t1 > t2)
			{
				std::swap(t1, t2);
			}

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
			{
				return false;
			}
		}
	}

	// If the intersection is behind the ray origin
	if (tMax < 0.0f)
	{
		return false;
	}

	return true;
}

bool	LibMath::intersectionRayPlane(Line3D ray, Plane plane, Vector3& outputPoint)
{
	float denom = plane.getNormal().dot(ray.getDirection());

	if (std::abs(denom) < g_epsilon) // ray is parallel
		return false;

	float t = (plane.getPointOnPlane() - ray.getCenter()).dot(plane.getNormal()) / denom;

	if (t < 0) // intersection behind ray origin
		return false;

	outputPoint = ray.getCenter() + ray.getDirection() * t;
	return true;
}

bool	LibMath::intersectionRayPrism(Line3D ray, Prism prism, Vector3& outputPoint)
{
	Point3D halfSize(prism.extentX(), prism.extentY(), prism.extentZ());

	LibMath::Vector3 minPoint = prism.getCenter() - halfSize;
	LibMath::Vector3 maxPoint = prism.getCenter() + halfSize;

	LibMath::Vector3 rayCenter(ray.getCenter().getX(), ray.getCenter().getY(), ray.getCenter().getZ());

	float tMin = -std::numeric_limits<float>::infinity();
	float tMax = std::numeric_limits<float>::infinity();

	for (int i = 0; i < 3; ++i)
	{
		float origin = rayCenter[i];
		float dir = ray.getDirection()[i];
		float minBound = minPoint[i];
		float maxBound = maxPoint[i];

		if (std::abs(dir) < g_epsilon)
		{
			// Ray is parallel to slab. No hit if origin not within slab
			if (origin < minBound || origin > maxBound)
			{
				return false;
			}
		}
		else
		{
			float invD = 1.0f / dir;
			float t1 = (minBound - origin) * invD;
			float t2 = (maxBound - origin) * invD;

			if (t1 > t2)
			{
				std::swap(t1, t2);
			}

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
			{
				return false;
			}
		}
	}

	// If the intersection is behind the ray origin
	if (tMax < 0.0f)
	{
		return false;
	}

	float t = tMin >= 0.0f ? tMin : tMax;
	outputPoint = ray.getCenter() + ray.getDirection() * t;

	return true;
}

bool	LibMath::collisionCapsulePrism(Capsule capsule, Prism prism)	// AABB collision only
{
	Point3D halfSize(prism.extentX(), prism.extentY(), prism.extentZ());

	LibMath::Vector3 minPoint = prism.getCenter() - halfSize;
	LibMath::Vector3 maxPoint = prism.getCenter() + halfSize;

	LibMath::Vector3 capLine = capsule.getEnd() - capsule.getStart();

	// Project the box center onto the capsule segment
	float abLengthSq = capLine.magnitudeSquared();
	float t = 0.0f;

	if (abLengthSq > 0.0f)
	{
		t = (prism.getCenter() - capsule.getStart()).dot(capLine) / abLengthSq;
		t = clamp(t, 0.0f, 1.0f);
	}

	Vector3 closestPointOnSegment = capsule.getStart() + capLine * t;

	// Clamp closest point to AABB
	Vector3 clampedPoint;
	clampedPoint[0] = clamp(closestPointOnSegment[0], minPoint[0], maxPoint[0]);
	clampedPoint[1] = clamp(closestPointOnSegment[1], minPoint[1], maxPoint[1]);
	clampedPoint[2] = clamp(closestPointOnSegment[2], minPoint[2], maxPoint[2]);

	// Compute squared distance
	Vector3 delta = closestPointOnSegment - clampedPoint;
	float distSq = delta.dot(delta);

	return distSq <= capsule.getRadius() * capsule.getRadius();
}

bool	LibMath::intersectionCapsulePrism(Capsule capsule, Prism prism, Vector3& outCorrection)	// AABB collision only
{
	Point3D halfSize(prism.extentX(), prism.extentY(), prism.extentZ());

	LibMath::Vector3 minPoint = prism.getCenter() - halfSize;
	LibMath::Vector3 maxPoint = prism.getCenter() + halfSize;

	LibMath::Vector3 capLine = capsule.getEnd() - capsule.getStart();

	// Project the box center onto the capsule segment
	float abLengthSq = capLine.magnitudeSquared();
	float t = 0.0f;

	if (abLengthSq > 0.0f)
	{
		t = (prism.getCenter() - capsule.getStart()).dot(capLine) / abLengthSq;
		t = clamp(t, 0.0f, 1.0f);
	}

	Vector3 closestPointOnSegment = capsule.getStart() + capLine * t;

	// Clamp closest point to AABB
	Vector3 clampedPoint;
	clampedPoint[0] = clamp(closestPointOnSegment[0], minPoint[0], maxPoint[0]);
	clampedPoint[1] = clamp(closestPointOnSegment[1], minPoint[1], maxPoint[1]);
	clampedPoint[2] = clamp(closestPointOnSegment[2], minPoint[2], maxPoint[2]);

	// Compute squared distance
	Vector3 delta = closestPointOnSegment - clampedPoint;
	float distSq = delta.dot(delta);

	// Check collision
	if (distSq <= capsule.getRadius() * capsule.getRadius())
	{
		float dist = std::sqrt(distSq);

		if (dist > 1e-6f)
		{
			// Penetration depth
			float penetrationDepth = capsule.getRadius() - dist;

			// Normalize delta and multiply by penetration depth for correction
			LibMath::Vector3 correction = delta * (penetrationDepth / dist);
			outCorrection = correction;
		}
		else
		{
			// If distance is zero (exact overlap), push up arbitrarily (e.g. up)
			outCorrection = LibMath::Vector3(0, capsule.getRadius(), 0);
		}

		return true;
	}

	// No collision
	outCorrection = LibMath::Vector3(0, 0, 0);
	return false;
}