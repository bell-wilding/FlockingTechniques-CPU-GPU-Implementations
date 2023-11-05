#pragma once

#include "Simulation.h"

namespace NCL {
	class Flock;

	class SettingsLoader {
	public:
		SettingsLoader() {};
		~SettingsLoader() {};

		Simulation::Settings LoadSettingsFromFile(std::string filename);

	};
}