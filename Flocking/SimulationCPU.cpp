#include "SimulationCPU.h"
#include "Flock.h"
#include <functional>
#include "../Common/Maths.h"
#include "../Common/Quaternion.h"
#include "../Common/Camera.h"

using namespace NCL;

SimulationCPU::SimulationCPU(bool octree, Simulation::Settings simSettings, FlockingRenderer* renderer) : Simulation(simSettings, renderer) {
	showOctree = false;
	useOctree = octree;

	octreeMaxDepth = 4;
	octreeMaxSize = 15;

	InitFlock();

	Debug::SetRenderer(renderer);
	renderer->InitFlock(bufFlock, numAgents, settings.maxBound, settings.modelScale);
}

void SimulationCPU::Update(float dt) {
	srand((int)(gameTime * 1000.0f));

	renderer->UpdateCamera(dt);

	UpdateStats(dt);
	UpdateKeys(dt);

	PerformFlock(dt);

	DrawUIText();
	renderer->DrawBoundingBox(drawBox ? flock->maxBound : 0);
	Debug::FlushRenderables(dt);
	renderer->Render();
}

void SimulationCPU::UpdateKeys(float dt) {
	Simulation::UpdateKeys(dt);

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
		useOctree = !useOctree;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::O)) {
		showOctree = !showOctree;
	}
}

void SimulationCPU::PerformFlock(float dt) {
	Octree tree(Vector3(1, 1, 1) * flock->maxBound, octreeMaxDepth, octreeMaxSize);

	if (useOctree) {
		for (int i = 0; i < numAgents; ++i) {
			tree.Insert((*flock)[i]);
		}
	}

	for (int i = 0; i < numAgents; ++i) {
		Agent* a = (*flock)[i];
		if (!paused) {
			if (useOctree) {
				FlockTree(a, tree, dt);
			}
			else {
				FlockBruteForce(a, flock->agentVector, dt);
			}
		}
		if (showRadii) {
			DrawRadii(a->position, flock->alignmentRadius, Debug::BLUE);
			DrawRadii(a->position, flock->separationRadius, Debug::GREEN);
			DrawRadii(a->position, flock->cohesionRadius, Debug::RED); 
		}
	}

	if (useOctree && showOctree) {
		tree.DebugDraw();
	}

	glBufferData(GL_SHADER_STORAGE_BUFFER, numAgents * sizeof(Agent), flock->agents, GL_DYNAMIC_COPY);
}

void SimulationCPU::FlockTree(Agent* a, Octree& tree, float dt) {
	std::vector<Agent*> neighbours;
	tree.GetNeighbours(a, flock->maxRadius, neighbours);
	FlockBruteForce(a, neighbours, dt);
}

void SimulationCPU::FlockBruteForce(Agent* a, std::vector<Agent*> neighbours, float dt) {
	Vector3 acceleration(0, 0, 0);

	acceleration += Steer(Alignment(a, neighbours), a->velocity) * flock->alignmentWeight;
	acceleration += Steer(Separation(a, neighbours), a->velocity) * flock->separationWeight;
	acceleration += Steer(Cohesion(a, neighbours), a->velocity) * flock->cohesionWeight;
	acceleration += Steer(InteractWithRay(a), a->velocity) * flock->avoidanceWeight;

	a->position += a->velocity * dt;
	a->velocity += acceleration;
	a->velocity = Vector3::ClampMagnitude(a->velocity, flock->maxVelocity);

	a->position = WrapBounds(a->position);
}

void NCL::SimulationCPU::AvoidWalls(Agent* a, float dt) {

	float turningDist = 25;
	float turningAngle = Maths::PI * 1.5f;

	auto GetPerpVector = [](Vector3 v, Vector3 n)->Vector3 {
		n.Normalise();
		Vector3 temp = n * Vector3::Dot(v, n);
		return (v - temp * 2);
	};

	Vector4 perpVec;
	Vector4 newVel;

	float distTop = flock->maxBound - a->position.y;
	float aTop = abs(acos(Vector3::Dot(Vector3(0, 1, 0), a->velocity.Normalised())));
	if (aTop < turningAngle) {
		perpVec = GetPerpVector(a->velocity, Vector3(0, 1, 0));
		a->velocity += perpVec.Normalised() * Maths::Clamp((1 - (distTop / turningDist)), 0.0f, 1.0f);
	}

	float distBottom = abs(-flock->maxBound - a->position.y);
	float aBottom = abs(acos(Vector3::Dot(Vector3(0, -1, 0), a->velocity.Normalised())));
	if (distBottom < turningDist && aBottom < turningAngle) {
		perpVec = GetPerpVector(a->velocity, Vector3(0, -1, 0));
		a->velocity += perpVec.Normalised() * Maths::Clamp((1 - (distBottom / turningDist)), 0.0f, 1.0f);
	}

	float distXFor = flock->maxBound - a->position.x;
	float aXFor = abs(acos(Vector3::Dot(Vector3(1, 0, 0), a->velocity.Normalised())));
	if (distXFor < turningDist) {
		perpVec = GetPerpVector(a->velocity, Vector3(1, 0, 0));
		a->velocity += perpVec.Normalised() * Maths::Clamp((1 - (distXFor / turningDist)), 0.0f, 1.0f);
	}

	float distXBack = abs(-flock->maxBound - a->position.x);
	float aXBack = abs(acos(Vector3::Dot(Vector3(-1, 0, 0), a->velocity.Normalised())));
	if (distXBack < turningDist) {
		perpVec = GetPerpVector(a->velocity, Vector3(-1, 0, 0));
		a->velocity += perpVec.Normalised() * Maths::Clamp((1 - (distXBack / turningDist)), 0.0f, 1.0f);
	}

	float distZFor = flock->maxBound - a->position.z;
	float aZFor = abs(acos(Vector3::Dot(Vector3(0, 0, 1), a->velocity.Normalised())));
	if (distZFor < turningDist) {
		perpVec = GetPerpVector(a->velocity, Vector3(0, 0, 1));
		a->velocity += perpVec.Normalised() * Maths::Clamp((1 - (distZFor / turningDist)) * dt * 100, 0.0f, 1.0f);
	}

	float distZBack = abs(-flock->maxBound - a->position.z);
	float aZBack = abs(acos(Vector3::Dot(Vector3(0, 0, -1), a->velocity.Normalised())));
	if (distZBack < turningDist) {
		perpVec = GetPerpVector(a->velocity, Vector3(0, 0, -1));
		a->velocity += perpVec.Normalised() * Maths::Clamp((1 - (distZBack / turningDist)) * dt * 100, 0.0f, 1.0f);
	}
}

Vector3 NCL::SimulationCPU::InteractWithRay(Agent* a) {
	bool attracting = Window::GetMouse()->ButtonDown(MouseButtons::RIGHT);
	if (Window::GetMouse()->ButtonDown(MouseButtons::LEFT) || attracting) {
		Camera* c = renderer->GetCamera();
		Ray r = Ray(c->GetPosition(), Quaternion::EulerAnglesToQuaternion(c->GetPitch(), c->GetYaw(), 0) * Vector3(0, 0, -1));

		Vector3 avoidancePoint = r.ClosestPointOnRay(a->position);

		float distance = (a->position - avoidancePoint).LengthSquared();

		if (distance < flock->avoidanceRadiusSquared) {
			float strength = 1.0f - (distance / flock->avoidanceRadiusSquared);
			return (a->position - avoidancePoint) * strength * (attracting ? 1 : -1);
		}
	}
	return Vector3(0, 0, 0);
}

Vector3 SimulationCPU::Steer(Vector3 desiredSteer, Vector3 velocity) {
	Vector3 steer = desiredSteer.Normalised() * flock->maxVelocity - velocity;
	steer = Vector3::ClampMagnitude(steer, flock->maxSteeringAngle);
	return steer;
}

bool NCL::SimulationCPU::WithinView(Agent* a, Agent* neighbour) {
	float cosP = cos(Maths::DegreesToRadians(90));

	float dx1 = a->velocity.x - a->position.x;
	float dy1 = a->velocity.y - a->position.y;
	float dz1 = a->velocity.z - a->position.z;
	float dx2 = neighbour->position.x - a->position.x;
	float dy2 = neighbour->position.y - a->position.y;
	float dz2 = neighbour->position.z - a->position.z;

	float cosTheta = (dx1 * dx2 + dy1 * dy2 + dz1 * dz2) / (sqrt(dx1 * dx1 + dy1 * dy1 + dz1 * dz1) * sqrt(dx2 * dx2 + dy2 * dy2 + dz2 * dz2));

	return cosTheta < cosP;
}

Vector3& SimulationCPU::WrapBounds(Vector3 position) {
	Vector3 newPos = position;

	if (position.x < -flock->maxBound)
	{
		newPos.x = flock->maxBound;
	}
	else if (position.x > flock->maxBound)
	{
		newPos.x = -flock->maxBound;
	}

	if (position.y < -flock->maxBound)
	{
		newPos.y = flock->maxBound;
	}
	else if (position.y > flock->maxBound)
	{
		newPos.y = -flock->maxBound;
	}

	if (position.z < -flock->maxBound)
	{
		newPos.z = flock->maxBound;
	}
	else if (position.z > flock->maxBound)
	{
		newPos.z = -flock->maxBound;
	}

	return newPos;
}

Vector3& SimulationCPU::Alignment(Agent* a, std::vector<Agent*> neighbours) {
	Vector3 steering = a->velocity;

	for (int i = 0; i < neighbours.size(); ++i) {
		Agent* neighbour = neighbours[i];
		if (a == neighbour) {
			continue;
		}
		float distance = (a->position - neighbour->position).LengthSquared();

		if (distance > flock->alignmentRadiusSquared) {
			continue;
		}
		steering += neighbour->velocity;
	}
	return steering;
}

Vector3& SimulationCPU::Separation(Agent* a, std::vector<Agent*> neighbours) {
	Vector3 steering = a->velocity;

	for (int i = 0; i < neighbours.size(); ++i) {
		Agent* neighbour = neighbours[i];
		if (a == neighbour) {
			continue;
		}
		float distance = (a->position - neighbour->position).LengthSquared();

		if (distance > flock->separationRadiusSquared) {
			continue;
		}
		float strength = 1.0f - (distance / flock->separationRadiusSquared);
		steering += (a->position - neighbour->position) * strength;
	}
	return steering;
}

Vector3& SimulationCPU::Cohesion(Agent* a, std::vector<Agent*> neighbours) {
	Vector3 steering = a->position;
	int neighbourCount = 1;

	for (int i = 0; i < neighbours.size(); ++i) {
		Agent* neighbour = neighbours[i];
		if (a == neighbour) {
			continue;
		}
		float distance = (a->position - neighbour->position).LengthSquared();

		if (distance > flock->cohesionRadiusSquared) {
			continue;
		}
		steering += neighbour->position;
		neighbourCount++;
	}

	steering /= neighbourCount;
	Vector3 vel = steering - a->position;
	return vel;
}


