#pragma once
#include "Vector3.h"
#include "Plane.h"
#include "Maths.h"

namespace NCL {
	namespace Maths {
		struct RayCollision {
			void* node;			//Node that was hit
			Vector3		collidedAt;		//WORLD SPACE position of the collision!
			float		rayDistance;

			RayCollision() {
				node = nullptr;
				rayDistance = FLT_MAX;
			}
		};

		class Ray {
		public:
			Ray(Vector3 position, Vector3 direction) {
				this->position = position;
				this->direction = direction;
			}
			~Ray(void) {}

			Vector3 GetPosition() const { return position; }

			Vector3 GetDirection() const { return direction; }

			Vector3 ClosestPointOnRay(const Vector3 point, float range = 100000) const {
				Vector3 lineEnd = position + direction.Normalised() * range;

				Vector3 rayLine = lineEnd - position;
				float t = Vector3::Dot(point - position, rayLine) / Vector3::Dot(rayLine, rayLine);
				return position + rayLine * Maths::Clamp(t, 0.0f, 1.0f);
			}

		protected:
			Vector3 position;	//World space position
			Vector3 direction;	//Normalised world space direction
		};
	}
}