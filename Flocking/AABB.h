#pragma once

#include "../Common/Vector3.h"

namespace NCL {
	class AABB {
	public:
		static bool Intersection(const Vector3& s1, const Vector3& p1, const Vector3& s2, const Vector3& p2) {
			Vector3 delta = p2 - p1;
			Vector3 totalSize = s1 + s2;

			return 
				abs(delta.x) < totalSize.x &&
				abs(delta.y) < totalSize.y &&
				abs(delta.z) < totalSize.z;
		}

		// Adapted from: https://stackoverflow.com/questions/28343716/sphere-intersection-test-of-aabb
		static bool SphereInsersection(const Vector3& s, const Vector3& p, const Vector3& sphereCenter, float sphereRadius) {
			float sphereRadSquared = sphereRadius * sphereRadius;
			Vector3 min = p - s;
			Vector3 max = p + s;

			if (sphereCenter.x < min.x)			sphereRadSquared -= (sphereCenter.x - min.x) * (sphereCenter.x - min.x);
			else if (sphereCenter.x > max.x)	sphereRadSquared -= (sphereCenter.x - max.x) * (sphereCenter.x - max.x);
			if (sphereCenter.y < min.y)			sphereRadSquared -= (sphereCenter.y - min.y) * (sphereCenter.y - min.y);
			else if (sphereCenter.y > max.y)	sphereRadSquared -= (sphereCenter.y - max.y) * (sphereCenter.y - max.y);
			if (sphereCenter.z < min.z)			sphereRadSquared -= (sphereCenter.z - min.z) * (sphereCenter.z - min.z);
			else if (sphereCenter.z > max.z)	sphereRadSquared -= (sphereCenter.z - max.z) * (sphereCenter.z - max.z);

			return sphereRadSquared > 0;
		}

		static bool PointContained(const Vector3& p, const Vector3& center, const Vector3& size) {
			return p.x >= center.x - size.x && p.x <= center.x + size.x
				&& p.y >= center.y - size.y && p.y <= center.y + size.y
				&& p.z >= center.z - size.z && p.z <= center.z + size.z;
		}

		static Vector3 GetHalfSizeFromRadius(float radius) {
			return Vector3(1, 1, 1) * radius;
		}
	};
}