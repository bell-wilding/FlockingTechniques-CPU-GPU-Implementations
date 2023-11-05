#include "SimulationGPU.h"
#include "Debug.h"
#include "Flock.h"
#include "../Common/Quaternion.h"
#include "../Common/Camera.h"
#include "../Plugins/OpenGLRendering/OGLComputeShader.h"

using namespace NCL;

SimulationGPU::SimulationGPU(bool useGrid, Simulation::Settings simSettings, FlockingRenderer* renderer) : Simulation(simSettings, renderer) {
	this->useGrid = useGrid;
	this->showGrid = false;

	gridVisualisationRange = 8000;

	InitFlock();

	invocations = 1024;
	workGroups = max(numAgents / invocations, 1);

	InitFlockComputer();
	InitGrid();

	Debug::SetRenderer(renderer);
	renderer->InitFlock(bufFlock, numAgents, settings.maxBound, settings.modelScale);
}

SimulationGPU::~SimulationGPU() {
	glDeleteBuffers(1, &bufFlock);
	glDeleteBuffers(1, &bufGridInd);

	delete bruteForceComputer;
	delete gridComputer;
	delete flockSorter;
}

void SimulationGPU::Update(float dt) {
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

void SimulationGPU::UpdateKeys(float dt) {
	Simulation::UpdateKeys(dt);

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
		useGrid = !useGrid;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::O)) {
		showGrid = !showGrid;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::I)) {
		showGridAtRange = !showGridAtRange;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::U)) {
		renderer->ToggleGridVisualisation();
	}
}

void NCL::SimulationGPU::PerformFlock(float dt) {
	if (!paused && useGrid) {
		FlockGrid(dt);
		BitonicSort();
	}
	else if (!paused) {
		FlockBruteForce(dt);
	}

	if (useGrid && showGrid) {
		DrawGrid();
	}
}

void NCL::SimulationGPU::DrawGrid() {
	int cellsPerAxis = (int)flock->maxBound / gridDimension + 1;

	for (int x = 0; x < cellsPerAxis; ++x) {
		for (int y = 0; y < cellsPerAxis; ++y) {
			for (int z = 0; z < cellsPerAxis; ++z) {
				Vector3 boxCenter((x * gridDimension * 2) + -flock->maxBound + gridDimension, (y * gridDimension * 2) + -flock->maxBound + gridDimension, (z * gridDimension * 2) + -flock->maxBound + gridDimension);
				if (showGridAtRange) {
					if ((renderer->GetCamera()->GetPosition() - boxCenter).LengthSquared() < gridVisualisationRange) {
						renderer->DrawBox(boxCenter, gridDimension);
					}
				}
				else {
					renderer->DrawBox(boxCenter, gridDimension);
				}
			}
		}
	}
}

void NCL::SimulationGPU::InitFlockComputer() {
	bruteForceComputer = new OGLComputeShader("FlockBruteForce.glsl");

	bruteForceComputer->Bind();
	glUniform1ui(glGetUniformLocation(bruteForceComputer->GetProgramID(), "size"), numAgents);
	glUniform1f(glGetUniformLocation(bruteForceComputer->GetProgramID(), "bound"), flock->maxBound);
	glUniform4f(glGetUniformLocation(bruteForceComputer->GetProgramID(), "radii"), flock->alignmentRadiusSquared, flock->separationRadiusSquared, flock->cohesionRadiusSquared, flock->avoidanceRadiusSquared);
	glUniform1f(glGetUniformLocation(bruteForceComputer->GetProgramID(), "maxVel"), flock->maxVelocity);
	glUniform1f(glGetUniformLocation(bruteForceComputer->GetProgramID(), "maxAngle"), flock->maxSteeringAngle);
	glUniform1i(glGetUniformLocation(bruteForceComputer->GetProgramID(), "mouseState"), 0);
}

void NCL::SimulationGPU::InitGrid() {
	gridComputer = new OGLComputeShader("FlockGrid.glsl");
	flockSorter = new OGLComputeShader("FlockSort.glsl");
	gridIndexer = new OGLComputeShader("GridIndex.glsl");

	gridDimension = flock->maxRadius * 0.5f;
	int cellsPerAxis = (int)flock->maxBound / gridDimension + 1;

	gridComputer->Bind();
	glUniform1ui(glGetUniformLocation(gridComputer->GetProgramID(), "size"), numAgents);
	glUniform1f(glGetUniformLocation(gridComputer->GetProgramID(), "bound"), flock->maxBound);
	glUniform4f(glGetUniformLocation(gridComputer->GetProgramID(), "radii"), flock->alignmentRadiusSquared, flock->separationRadiusSquared, flock->cohesionRadiusSquared, flock->avoidanceRadiusSquared);
	glUniform1f(glGetUniformLocation(gridComputer->GetProgramID(), "maxVel"), flock->maxVelocity);
	glUniform1f(glGetUniformLocation(gridComputer->GetProgramID(), "maxAngle"), flock->maxSteeringAngle);
	glUniform1i(glGetUniformLocation(gridComputer->GetProgramID(), "mouseState"), 0);

	glUniform1ui(glGetUniformLocation(gridComputer->GetProgramID(), "cellsPerAxis"), cellsPerAxis);
	glUniform1f(glGetUniformLocation(gridComputer->GetProgramID(), "cellDimensionReciprocal"), 1/(2 * gridDimension));

	flockSorter->Bind();
	glUniform1ui(glGetUniformLocation(flockSorter->GetProgramID(), "size"), numAgents);

	gridIndexer->Bind();
	glUniform1ui(glGetUniformLocation(gridIndexer->GetProgramID(), "size"), numAgents);
	glUniform1ui(glGetUniformLocation(gridIndexer->GetProgramID(), "cellsPerAxis"), cellsPerAxis);

	glGenBuffers(1, &bufGridInd);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufGridInd);
	glBufferData(GL_SHADER_STORAGE_BUFFER, cellsPerAxis * cellsPerAxis * cellsPerAxis * sizeof(uint32_t), nullptr, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridInd);
}

// Adapted from: https://github.com/vojtatom/flocking-cpp/blob/master/src/glcontext.cpp
void NCL::SimulationGPU::BitonicSort() {
	flockSorter->Bind();

	int segmentSize = 2;
	int iteration = 0;
	int sizeOfIndependentBlock;

	do
	{
		do
		{
			glUniform1ui(glGetUniformLocation(flockSorter->GetProgramID(), "segmentSize"), segmentSize);
			glUniform1ui(glGetUniformLocation(flockSorter->GetProgramID(), "iteration"), iteration);
			flockSorter->Execute(workGroups, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			sizeOfIndependentBlock = segmentSize / int(pow(2, iteration));
			iteration++;

		} while (sizeOfIndependentBlock > invocations);

		iteration = 0;
		segmentSize *= 2;
	} while (segmentSize <= flock->size);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	gridIndexer->Bind();
	gridIndexer->Execute(workGroups, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	gridIndexer->Unbind();
}

void NCL::SimulationGPU::FlockBruteForce(float dt) {
	bruteForceComputer->Bind();

	CastAvoidanceRay();

	glUniform1f(glGetUniformLocation(bruteForceComputer->GetProgramID(), "dt"), dt);
	glUniform4f(glGetUniformLocation(bruteForceComputer->GetProgramID(), "weights"), flock->alignmentWeight, flock->separationWeight, flock->cohesionWeight, flock->avoidanceWeight);

	bruteForceComputer->Execute(workGroups, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	bruteForceComputer->Unbind();
}

void NCL::SimulationGPU::FlockGrid(float dt) {
	gridComputer->Bind();

	CastAvoidanceRay();

	glUniform1f(glGetUniformLocation(gridComputer->GetProgramID(), "dt"), dt);
	glUniform4f(glGetUniformLocation(gridComputer->GetProgramID(), "weights"), flock->alignmentWeight, flock->separationWeight, flock->cohesionWeight, flock->avoidanceWeight);

	gridComputer->Execute(workGroups, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	gridComputer->Unbind();
}

void NCL::SimulationGPU::CastAvoidanceRay() {
	bool attracting = Window::GetMouse()->ButtonDown(MouseButtons::RIGHT);
	if (Window::GetMouse()->ButtonDown(MouseButtons::LEFT) || attracting) {
		Camera* c = renderer->GetCamera();
		Ray r = Ray(c->GetPosition(), Quaternion::EulerAnglesToQuaternion(c->GetPitch(), c->GetYaw(), 0) * Vector3(0, 0, -1));

		Vector3 lineStart = r.GetPosition();
		Vector3 lineEnd = r.GetPosition() + r.GetDirection().Normalised() * 100000;

		glUniform3fv(glGetUniformLocation(bruteForceComputer->GetProgramID(), "avLineStart"), 1, (float*)&lineStart);
		glUniform3fv(glGetUniformLocation(bruteForceComputer->GetProgramID(), "avLineEnd"), 1, (float*)&lineEnd);
		glUniform1i(glGetUniformLocation(bruteForceComputer->GetProgramID(), "mouseState"), attracting ? 1 : 2);
	}
	else {
		glUniform1i(glGetUniformLocation(bruteForceComputer->GetProgramID(), "mouseState"), 0);
	}
}
