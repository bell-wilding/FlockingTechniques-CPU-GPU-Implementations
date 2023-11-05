#include "Octree.h"

void NCL::OctreeNode::Insert(Agent* object, int depthLeft, int maxSize) {
	if (!AABB::PointContained(object->position, position, size)) {
		return;
	}
	if (children) {
		for (int i = 0; i < 8; ++i) {
			children[i].Insert(object, depthLeft - 1, maxSize);
		}
	}
	else {
		contents.push_back(object);
		if (contents.size() > maxSize && depthLeft > 0) {
			if (!children) {
				Split();
				for (Agent* a : contents) {
					for (int j = 0; j < 8; ++j) {
						children[j].Insert(a, depthLeft - 1, maxSize);
					}
				}
				contents.clear();
			}
		}
	}
}

void NCL::OctreeNode::GetNeighbours(Agent* object, float radius, std::vector<Agent*>& collidingNodes, bool useSphereOverlap) {
	bool overlap = useSphereOverlap ?
		AABB::SphereInsersection(size, position, object->position, radius) :
		AABB::Intersection(AABB::GetHalfSizeFromRadius(radius), object->position, size, position);

	if (!overlap) {
		return;
	}
	if (children) {
		for (int i = 0; i < 8; ++i) {
			children[i].GetNeighbours(object, radius, collidingNodes);
		}
	}
	else {
		for (Agent* a : contents) {
			collidingNodes.push_back(a);
		}
	}
}

void NCL::OctreeNode::Split() {
	Vector3 halfSize = size * 0.5f;
	children = new OctreeNode[8];
	children[0] = OctreeNode(position + Vector3(-halfSize.x, halfSize.y, halfSize.z), halfSize);
	children[1] = OctreeNode(position + Vector3(halfSize.x, halfSize.y, halfSize.z), halfSize);
	children[2] = OctreeNode(position + Vector3(-halfSize.x, -halfSize.y, halfSize.z), halfSize);
	children[3] = OctreeNode(position + Vector3(halfSize.x, -halfSize.y, halfSize.z), halfSize);
	children[4] = OctreeNode(position + Vector3(-halfSize.x, halfSize.y, -halfSize.z), halfSize);
	children[5] = OctreeNode(position + Vector3(halfSize.x, halfSize.y, -halfSize.z), halfSize);
	children[6] = OctreeNode(position + Vector3(-halfSize.x, -halfSize.y, -halfSize.z), halfSize);
	children[7] = OctreeNode(position + Vector3(halfSize.x, -halfSize.y, -halfSize.z), halfSize);
}

void NCL::OctreeNode::DebugDraw() {
	Debug::DrawLine(position + Vector3(1, 1, 1) * size, position + Vector3(1, 1, -1) * size, Debug::GetLineColour());
	Debug::DrawLine(position + Vector3(1, 1, -1) * size, position + Vector3(-1, 1, -1) * size, Debug::GetLineColour());
	Debug::DrawLine(position + Vector3(-1, 1, -1) * size, position + Vector3(-1, 1, 1) * size, Debug::GetLineColour());
	Debug::DrawLine(position + Vector3(-1, 1, 1) * size, position + Vector3(1, 1, 1) * size, Debug::GetLineColour());

	Debug::DrawLine(position + Vector3(1, -1, 1) * size, position + Vector3(1, -1, -1) * size, Debug::GetLineColour());
	Debug::DrawLine(position + Vector3(1, -1, -1) * size, position + Vector3(-1, -1, -1) * size, Debug::GetLineColour());
	Debug::DrawLine(position + Vector3(-1, -1, -1) * size, position + Vector3(-1, -1, 1) * size, Debug::GetLineColour());
	Debug::DrawLine(position + Vector3(-1, -1, 1) * size, position + Vector3(1, -1, 1) * size, Debug::GetLineColour());

	Debug::DrawLine(position + Vector3(1, -1, 1) * size, position + Vector3(1, 1, 1) * size, Debug::GetLineColour());
	Debug::DrawLine(position + Vector3(1, -1, -1) * size, position + Vector3(1, 1, -1) * size, Debug::GetLineColour());
	Debug::DrawLine(position + Vector3(-1, -1, -1) * size, position + Vector3(-1, 1, -1) * size, Debug::GetLineColour());
	Debug::DrawLine(position + Vector3(-1, -1, 1) * size, position + Vector3(-1, 1, 1) * size, Debug::GetLineColour());

	if (children) {
		for (int i = 0; i < 8; ++i) {
			children[i].DebugDraw();
		}
	}
}
