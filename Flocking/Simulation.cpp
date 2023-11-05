#include "Simulation.h"
#include "Debug.h"
#include "Flock.h"
#include <random>
#include <ctime>
#include <chrono>
#include <iomanip>
#include "../Plugins/FreeImage/FreeImage.h"

NCL::Simulation::Simulation(Settings simSettings, FlockingRenderer* renderer) {
	settings = simSettings;
	numAgents = settings.numAgents;

	fpsSmoothing = 0.95f;
	gameTime = 0;
	dtPrev = 0;

	this->renderer = renderer;
	flock = nullptr;
	bufFlock = -1;

	showRadii = false;
	paused = false;
	drawBox = true;
}

NCL::Simulation::~Simulation() {
	delete renderer;
	delete flock;
}

void NCL::Simulation::InitFlock() {
	Agent* agents = new Agent[numAgents];

	float maxRadius = std::fmax(settings.alignmentRadius, std::fmax(settings.separationRadius, settings.cohesionRadius));
	float cellDimensionReciprocal = 1 / maxRadius * 0.5f;
	int cellsPerAxis = (int)settings.maxBound / (maxRadius * 0.5f) + 1;

	for (int i = 0; i < numAgents; ++i) {
		float x = ((float)rand() / (float)(RAND_MAX / 2) - 1) * settings.maxBound;
		float y = ((float)rand() / (float)(RAND_MAX / 2) - 1) * settings.maxBound;
		float z = ((float)rand() / (float)(RAND_MAX / 2) - 1) * settings.maxBound;
		agents[i].position = Vector3(x, y, z);
		agents[i].velocity = Vector3((float)rand() / (float)(RAND_MAX / 2) - 1, (float)rand() / (float)(RAND_MAX / 2) - 1, (float)rand() / (float)(RAND_MAX / 2) - 1).Normalised() * settings.maxVelocity;
		agents[i].cell = int((agents[i].position.x + settings.maxBound) * cellDimensionReciprocal) + int((agents[i].position.y + settings.maxBound) * cellDimensionReciprocal) * cellsPerAxis + int((agents[i].position.z + settings.maxBound) * cellDimensionReciprocal) * cellsPerAxis * cellsPerAxis;
	}

	flock = new Flock(agents, settings);

	glGenBuffers(1, &bufFlock);
	glBindBuffer(GL_ARRAY_BUFFER, bufFlock);
	glBufferData(GL_ARRAY_BUFFER, numAgents * sizeof(Agent), nullptr, GL_DYNAMIC_COPY);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufFlock);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numAgents * sizeof(Agent), flock->agents, GL_DYNAMIC_COPY);
}

void NCL::Simulation::UpdateKeys(float dt) {
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM1)) {
		flock->alignmentWeight += 0.1 * dt;
		flock->alignmentWeight = fmin(1.0f, flock->alignmentWeight);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM2)) {
		flock->alignmentWeight -= 0.1 * dt;
		flock->alignmentWeight = fmax(0.0f, flock->alignmentWeight);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM3)) {
		flock->separationWeight += 0.1 * dt;
		flock->separationWeight = fmin(1.0f, flock->separationWeight);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM4)) {
		flock->separationWeight -= 0.1 * dt;
		flock->separationWeight = fmax(0.0f, flock->separationWeight);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
		flock->cohesionWeight += 0.1 * dt;
		flock->cohesionWeight = fmin(1.0f, flock->cohesionWeight);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM6)) {
		flock->cohesionWeight -= 0.1 * dt;
		flock->cohesionWeight = fmax(0.0f, flock->cohesionWeight);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		SaveScreen();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::C)) {
		showRadii = !showRadii;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
		paused = !paused;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K)) {
		renderer->ToggleDarkMode();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::L)) {
		renderer->ToggleLighting();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B)) {
		drawBox = !drawBox;
	}
}

void NCL::Simulation::UpdateStats(float dt) {
	gameTime += dt;
	dtPrev = (dtPrev * fpsSmoothing) + (dt * (1.0 - fpsSmoothing));

	srand((int)(gameTime * 1000.0f));
}

void NCL::Simulation::DrawUIText() {
	renderer->DrawString("FPS: " + std::to_string(1 / dtPrev),
		Vector2(1, 2), Vector4(0.5f, 0.3f, 0.8f, 1), 10.0f);

	renderer->DrawString("Simulation Time: " + std::to_string(gameTime),
		Vector2(40, 2), Vector4(0.5f, 0.3f, 0.8f, 1), 10.0f);

	renderer->DrawString("Num Agents: " + std::to_string(numAgents),
		Vector2(82, 2), Vector4(0.5f, 0.3f, 0.8f, 1), 10.0f);

	renderer->DrawString("[+1 | -2] Alignment: " + std::to_string(flock->alignmentWeight),
		Vector2(1, 99), Vector4(0.5f, 0.3f, 0.8f, 1), 10.0f);

	renderer->DrawString("[+3 | -4] Separation: " + std::to_string(flock->separationWeight),
		Vector2(37, 99), Vector4(0.5f, 0.3f, 0.8f, 1), 10.0f);

	renderer->DrawString("[+5 | -6] Cohesion: " + std::to_string(flock->cohesionWeight),
		Vector2(73, 99), Vector4(0.5f, 0.3f, 0.8f, 1), 10.0f);
}

void NCL::Simulation::DrawRadii(const Vector3& center, float radius, const Vector4& colour) {
	Vector3 points[42] = {
		center + Vector3(0, 1, 0) * radius,

		center + Vector3(0.25, 0.75, 0).Normalised() * radius,
		center + Vector3(0.5, 0.5, 0).Normalised() * radius,
		center + Vector3(0.75, 0.25, 0).Normalised() * radius,
		center + Vector3(1, 0, 0) * radius,

		center + Vector3(-0.25, 0.75, 0).Normalised() * radius,
		center + Vector3(-0.5, 0.5, 0).Normalised() * radius,
		center + Vector3(-0.75, 0.25, 0).Normalised() * radius,
		center + Vector3(-1, 0, 0) * radius,

		center + Vector3(0, 0.75, 0.25).Normalised() * radius,
		center + Vector3(0, 0.5, 0.5).Normalised() * radius,
		center + Vector3(0, 0.25, 0.75).Normalised() * radius,
		center + Vector3(0, 0, 1) * radius,

		center + Vector3(0, 0.75, -0.25).Normalised() * radius,
		center + Vector3(0, 0.5, -0.5).Normalised() * radius,
		center + Vector3(0, 0.25, -0.75).Normalised() * radius,
		center + Vector3(0, 0, -1) * radius,

		center + Vector3(0, -1, 0) * radius,

		center + Vector3(0.25, -0.75, 0).Normalised() * radius,
		center + Vector3(0.5, -0.5, 0).Normalised() * radius,
		center + Vector3(0.75, -0.25, 0).Normalised() * radius,

		center + Vector3(-0.25, -0.75, 0).Normalised() * radius,
		center + Vector3(-0.5, -0.5, 0).Normalised() * radius,
		center + Vector3(-0.75, -0.25, 0).Normalised() * radius,

		center + Vector3(0, -0.75, 0.25).Normalised() * radius,
		center + Vector3(0, -0.5, 0.5).Normalised() * radius,
		center + Vector3(0, -0.25, 0.75).Normalised() * radius,

		center + Vector3(0, -0.75, -0.25).Normalised() * radius,
		center + Vector3(0, -0.5, -0.5).Normalised() * radius,
		center + Vector3(0, -0.25, -0.75).Normalised() * radius,


		center + Vector3(0.25, 0, -0.75).Normalised() * radius,
		center + Vector3(0.5, 0, -0.5).Normalised() * radius,
		center + Vector3(0.75, 0, -0.25).Normalised() * radius,

		center + Vector3(-0.25, 0, -0.75).Normalised() * radius,
		center + Vector3(-0.5, 0, -0.5).Normalised() * radius,
		center + Vector3(-0.75, 0, -0.25).Normalised() * radius,

		center + Vector3(-0.75, 0, 0.25).Normalised() * radius,
		center + Vector3(-0.5, 0, 0.5).Normalised() * radius,
		center + Vector3(-0.25, 0, 0.75).Normalised() * radius,

		center + Vector3(0.75, 0, 0.25).Normalised() * radius,
		center + Vector3(0.5, 0, 0.5).Normalised() * radius,
		center + Vector3(0.25, 0, 0.75).Normalised() * radius,
	};

	Debug::DrawLine(points[0], points[1], colour);
	Debug::DrawLine(points[1], points[2], colour);
	Debug::DrawLine(points[2], points[3], colour);
	Debug::DrawLine(points[3], points[4], colour);

	Debug::DrawLine(points[0], points[5], colour);
	Debug::DrawLine(points[5], points[6], colour);
	Debug::DrawLine(points[6], points[7], colour);
	Debug::DrawLine(points[7], points[8], colour);

	Debug::DrawLine(points[0], points[9], colour);
	Debug::DrawLine(points[9], points[10], colour);
	Debug::DrawLine(points[10], points[11], colour);
	Debug::DrawLine(points[11], points[12], colour);

	Debug::DrawLine(points[0], points[13], colour);
	Debug::DrawLine(points[13], points[14], colour);
	Debug::DrawLine(points[14], points[15], colour);
	Debug::DrawLine(points[15], points[16], colour);

	Debug::DrawLine(points[17], points[18], colour);
	Debug::DrawLine(points[18], points[19], colour);
	Debug::DrawLine(points[19], points[20], colour);
	Debug::DrawLine(points[20], points[4], colour);

	Debug::DrawLine(points[17], points[21], colour);
	Debug::DrawLine(points[21], points[22], colour);
	Debug::DrawLine(points[22], points[23], colour);
	Debug::DrawLine(points[23], points[8], colour);

	Debug::DrawLine(points[17], points[24], colour);
	Debug::DrawLine(points[24], points[25], colour);
	Debug::DrawLine(points[25], points[26], colour);
	Debug::DrawLine(points[26], points[12], colour);

	Debug::DrawLine(points[17], points[27], colour);
	Debug::DrawLine(points[27], points[28], colour);
	Debug::DrawLine(points[28], points[29], colour);
	Debug::DrawLine(points[29], points[16], colour);

	Debug::DrawLine(points[16], points[30], colour);
	Debug::DrawLine(points[30], points[31], colour);
	Debug::DrawLine(points[31], points[32], colour);
	Debug::DrawLine(points[32], points[4], colour);

	Debug::DrawLine(points[16], points[33], colour);
	Debug::DrawLine(points[33], points[34], colour);
	Debug::DrawLine(points[34], points[35], colour);
	Debug::DrawLine(points[35], points[8], colour);

	Debug::DrawLine(points[8], points[36], colour);
	Debug::DrawLine(points[36], points[37], colour);
	Debug::DrawLine(points[37], points[38], colour);
	Debug::DrawLine(points[38], points[12], colour);

	Debug::DrawLine(points[4], points[39], colour);
	Debug::DrawLine(points[39], points[40], colour);
	Debug::DrawLine(points[40], points[41], colour);
	Debug::DrawLine(points[41], points[12], colour);
}

void NCL::Simulation::SaveScreen() {
	int width = renderer->GetWindowDimensions().x;
	int height = renderer->GetWindowDimensions().y;

	BYTE* pixels = new BYTE[3 * width * height];

	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, height, 3 * width, 24, 0x000FF0, 0xFF0000, 0x00FF00, false); //0x000FF, 0xFF0000, 0x00FF00,

	auto start = std::chrono::system_clock::now();
	auto legacyStart = std::chrono::system_clock::to_time_t(start);
	char tmBuff[30];
	ctime_s(tmBuff, sizeof(tmBuff), &legacyStart);

	string s = tmBuff;
	s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
	std::replace(s.begin(), s.end(), ':', '-');
	string filelocation = "../Screenshots/" + s + ".bmp";
	FreeImage_Save(FIF_BMP, image, filelocation.c_str(), 0);
	std::cout << "Screenshot saved: " << filelocation << "\n";

	FreeImage_Unload(image);
	delete[] pixels;
}

