#pragma once
#include "Debug.h"
#include "AABB.h"
#include "Agent.h"
#include <list>

namespace NCL {
	using namespace NCL::Maths;
	class Octree;
	class Agent;

	class OctreeNode {
	protected:
		friend class Octree;

		OctreeNode() {}

		OctreeNode(Vector3 pos, Vector3 size) {
			children = nullptr;
			this->position = pos;
			this->size = size;
		}

		~OctreeNode() {
			delete[] children;
		}

		void Insert(Agent* object, int depthLeft, int maxSize);
		void GetNeighbours(Agent* object, float radius, std::vector<Agent*>& collidingNodes, bool useSphereOverlap = false);
		void Split();
		void DebugDraw();

	protected:
		std::list<Agent*> contents;

		Vector3 position;
		Vector3 size;

		OctreeNode* children;
	};
}

namespace NCL {
	using namespace NCL::Maths;
	class Octree
	{
	public:
		Octree(Vector3 size, int maxDepth = 6, int maxSize = 5) {
			root = OctreeNode(Vector3(), size);
			this->maxDepth = maxDepth;
			this->maxSize = maxSize;
		}
		~Octree() {
		}

		void Insert(Agent* object) {
			root.Insert(object, maxDepth, maxSize);
		}

		void GetNeighbours(Agent* object, float radius, std::vector<Agent*>& collidingNodes, bool useSphereOverlap = false) {
			root.GetNeighbours(object, radius, collidingNodes, useSphereOverlap);
		}

		void DebugDraw() {
			root.DebugDraw();
		}

	protected:
		OctreeNode root;
		int maxDepth;
		int maxSize;
	};
}
