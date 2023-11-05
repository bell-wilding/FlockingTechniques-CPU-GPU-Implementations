#pragma once

#include "Agent.h"
#include "Simulation.h"
#include <vector>

namespace NCL {
	class Flock {
	public:
		Flock(Agent* agents, Simulation::Settings settings) {

			alignmentWeight = settings.alignmentWeight;
			separationWeight = settings.separationWeight;
			cohesionWeight = settings.cohesionWeight;
			avoidanceWeight = settings.avoidanceWeight;

			alignmentRadius = settings.alignmentRadius;
			separationRadius = settings.separationRadius;
			cohesionRadius = settings.cohesionRadius;
			avoidanceRadius = settings.avoidanceRadius;

			alignmentRadiusSquared = alignmentRadius * alignmentRadius;
			separationRadiusSquared = separationRadius * separationRadius;
			cohesionRadiusSquared = cohesionRadius * cohesionRadius;
			avoidanceRadiusSquared = avoidanceRadius * avoidanceRadius;

			maxRadius = std::fmax(alignmentRadius, std::fmax(separationRadius, cohesionRadius));
			maxRadiusSquared = fmax(alignmentRadiusSquared, std::fmax(separationRadiusSquared, cohesionRadiusSquared));

			maxVelocity = settings.maxVelocity;
			maxSteeringAngle = settings.maxSteeringAngle;

			maxBound = settings.maxBound;
			size = settings.numAgents;

			this->agents = agents;

			for (int i = 0; i < size; ++i) {
				agentVector.emplace_back(&agents[i]);
			}
		}

		~Flock() {
			delete[] agents;
			agents = nullptr;
		}

		int Size() const {
			return size;
		}

		Agent* operator[](const int index) const {
			if (index > size) {
				std::cout << "Index value out of range";
				exit(0);
			}
			return &agents[index];
		}

	protected:

		int size;
		Agent* agents;

		std::vector<Agent*> agentVector;

		float alignmentWeight;
		float separationWeight;
		float cohesionWeight;
		float avoidanceWeight;

		float alignmentRadius;
		float separationRadius;
		float cohesionRadius;
		float avoidanceRadius;

		float alignmentRadiusSquared;
		float separationRadiusSquared;
		float cohesionRadiusSquared;
		float avoidanceRadiusSquared;

		float maxRadius;
		float maxRadiusSquared;

		float maxVelocity;
		float maxSteeringAngle;

		float maxBound;

		friend class Simulation;
		friend class SimulationCPU;
		friend class SimulationGPU;
		friend class FlockingRenderer;
	};
}