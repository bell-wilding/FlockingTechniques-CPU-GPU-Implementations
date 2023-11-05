#pragma once
#include "FlockingRenderer.h"

namespace NCL {
	class Flock;

	class Simulation {
	public:
		struct Settings {
			int numAgents;

			float alignmentWeight;
			float separationWeight;
			float cohesionWeight;
			float avoidanceWeight;

			float alignmentRadius;
			float separationRadius;
			float cohesionRadius;
			float avoidanceRadius;

			float maxVelocity;
			float maxSteeringAngle;

			float maxBound;
			float modelScale;
		};

		Simulation(Settings simSettings, FlockingRenderer* renderer);
		~Simulation();

		virtual void Update(float dt) = 0;

	protected:

		void InitFlock();

		virtual void PerformFlock(float dt) = 0;
		virtual void UpdateKeys(float dt);

		void UpdateStats(float dt);

		void DrawUIText();
		void DrawRadii(const Vector3& position, float radius, const Vector4& colour = Vector4(1, 1, 1, 1));

		void SaveScreen();

		FlockingRenderer* renderer;
		GLuint bufFlock;

		float gameTime;
		float fpsSmoothing;
		float dtPrev;

		bool showRadii;
		bool paused;
		bool drawBox;

		int numAgents;
		Flock* flock;

		Settings settings;
	};
}