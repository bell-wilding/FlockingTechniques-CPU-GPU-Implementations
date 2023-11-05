#pragma once

#include "Simulation.h"
#include "Octree.h"
#include <vector>

namespace NCL {
	class Flock;

	class SimulationCPU : public Simulation {
	public:
		SimulationCPU(bool useOctree, Simulation::Settings simSettings, FlockingRenderer* renderer);
		~SimulationCPU() {};

		void Update(float dt) override;

	protected:

		void UpdateKeys(float dt) override;

		void PerformFlock(float dt) override;

		void FlockTree(Agent* a, Octree& tree, float dt);
		void FlockBruteForce(Agent* b, std::vector<Agent*> neighbours, float dt);

		void AvoidWalls(Agent* a, float dt);
		Vector3 InteractWithRay(Agent* b);

		Vector3 Steer(Vector3 desiredSteer, Vector3 velocity);
		bool WithinView(Agent* a, Agent* neighbour);
		Vector3& WrapBounds(Vector3 position);

		Vector3& Alignment(Agent* a, std::vector<Agent*> neighbours);
		Vector3& Separation(Agent* a, std::vector<Agent*> neighbours);
		Vector3& Cohesion(Agent* a, std::vector<Agent*> neighbours);

		bool showOctree;
		bool useOctree;

		int octreeMaxDepth;
		int octreeMaxSize;
	};
}

