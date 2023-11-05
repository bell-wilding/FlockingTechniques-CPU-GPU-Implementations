#include "../Common/Window.h"

#include "SimulationCPU.h"
#include "SimulationGPU.h"
#include "FlockingRenderer.h"
#include "SettingsLoader.h"

using namespace NCL;
using namespace std;

enum SimulationType {
	CPU_BRUTE_FORCE,
	CPU_OCTREE,
	GPU_BRUTE_FORCE,
	GPU_GRID
};

SimulationType SimulationMenu(FlockingRenderer* renderer) {
	bool cont = true;
	SimulationType type = CPU_BRUTE_FORCE;

	while (cont) {

		Window::GetWindow()->UpdateWindow();

		renderer->DrawString("Select a Simulation:", Vector2(32.5, 30), Vector4(0, 0, 0, 1));
		renderer->DrawString("1. CPU Brute Force", Vector2(35, 50), Vector4(1, 0.5, 0, 1), 17);
		renderer->DrawString("2. CPU Octree", Vector2(35, 55), Vector4(0, 0.8, 0, 1), 17);
		renderer->DrawString("3. GPU Brute Force", Vector2(35, 60), Vector4(0.5, 0, 0.5, 1), 17);
		renderer->DrawString("4. GPU Grid", Vector2(35, 65), Vector4(0, 0.5, 1, 1), 17);

		renderer->Render();

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
			type = CPU_BRUTE_FORCE;
			cont = false;
		} else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
			type = CPU_OCTREE;
			cont = false;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM3)) {
			type = GPU_BRUTE_FORCE;
			cont = false;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM4)) {
			type = GPU_GRID;
			cont = false;
		} else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
			exit(0);
		}
	}
	return type;
}

int main() {
	Window*w = Window::CreateGameWindow("CPU vs GPU Flocking Project", 1920, 1080);

	if (!w->HasInitialised()) {
		return -1;
	}
	srand(time(0));
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);
	w->SetWindowPosition(0, 0);

	SettingsLoader loader;
	FlockingRenderer* renderer = new FlockingRenderer();
	SimulationType selectedSimulation = SimulationMenu(renderer);

	Simulation* sim;
	if (selectedSimulation == CPU_OCTREE) {
		sim = new SimulationCPU(true, loader.LoadSettingsFromFile("SimSettingsCPU-Octree.txt"), renderer);
	}
	else if (selectedSimulation == GPU_BRUTE_FORCE) {
		sim = new SimulationGPU(false, loader.LoadSettingsFromFile("SimSettingsGPU-BruteForce.txt"), renderer);
	}
	else if (selectedSimulation == GPU_GRID) {
		sim = new SimulationGPU(true, loader.LoadSettingsFromFile("SimSettingsGPU-Grid.txt"), renderer);
	}
	else {
		sim = new SimulationCPU(false, loader.LoadSettingsFromFile("SimSettingsCPU-BruteForce.txt"), renderer);
	}

	w->GetTimer()->GetTimeDeltaSeconds();
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		w->SetTitle("Frame Time: " + std::to_string(1000.0f * dt));

		sim->Update(dt);
	}
	Window::DestroyGameWindow();
}
