#pragma once

#include "Simulation.h"

namespace NCL {
	class Flock;
	class OGLComputeShader;

	class SimulationGPU : public Simulation {
	public:
		SimulationGPU(bool useGrid, Simulation::Settings simSettings, FlockingRenderer* renderer);
		~SimulationGPU();

		void Update(float dt) override;

	protected:

		void UpdateKeys(float dt) override;

		void PerformFlock(float dt) override;

		void DrawGrid();

		void InitFlockComputer();
		void InitGrid();

		void BitonicSort();

		void FlockBruteForce(float dt);
		void FlockGrid(float dt);

		void CastAvoidanceRay();

		OGLComputeShader* bruteForceComputer = nullptr;

		OGLComputeShader* flockSorter = nullptr;
		OGLComputeShader* gridComputer = nullptr;
		OGLComputeShader* gridIndexer = nullptr;

		GLuint bufGridInd;

		int invocations;
		int workGroups;

		float gridDimension;

		bool useGrid;
		bool showGrid;
		bool showGridAtRange;

		float gridVisualisationRange;
	};
}