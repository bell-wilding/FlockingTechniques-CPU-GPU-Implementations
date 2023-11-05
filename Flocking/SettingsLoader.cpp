#include "SettingsLoader.h"
#include "../Common/Assets.h"
#include <sstream>

NCL::Simulation::Settings NCL::SettingsLoader::LoadSettingsFromFile(std::string filename) {
	Simulation::Settings settings;

	settings.numAgents = 2560;
	settings.maxBound = 50.0f;
	settings.maxVelocity = 20.0f;
	settings.maxSteeringAngle = 10.0f;
	settings.alignmentRadius = 2.8f;
	settings.separationRadius = 3.2f;
	settings.cohesionRadius = 3.5f;
	settings.avoidanceRadius = 22.4f;
	settings.alignmentWeight = 0.27f;
	settings.separationWeight = 0.96f;
	settings.cohesionWeight = 0.25f;
	settings.avoidanceWeight = 1.0f;
	settings.modelScale = 50.0f;

	std::string contents;

	if (Assets::ReadTextFile(Assets::DATADIR + filename, contents)) {

		std::istringstream iss(contents);
		std::string data;

		std::getline(iss, data);
		settings.numAgents = std::stoi(data);

		std::getline(iss, data);
		settings.maxBound = std::stof(data);

		std::getline(iss, data);
		settings.maxVelocity = std::stof(data);

		std::getline(iss, data);
		settings.maxSteeringAngle = std::stof(data);

		std::getline(iss, data);
		settings.alignmentRadius = std::stof(data);

		std::getline(iss, data);
		settings.separationRadius = std::stof(data);

		std::getline(iss, data);
		settings.cohesionRadius = std::stof(data);

		std::getline(iss, data);
		settings.avoidanceRadius = std::stof(data);

		std::getline(iss, data);
		settings.alignmentWeight = std::stof(data);

		std::getline(iss, data);
		settings.separationWeight = std::stof(data);

		std::getline(iss, data);
		settings.cohesionWeight = std::stof(data);

		std::getline(iss, data);
		settings.avoidanceWeight = std::stof(data);

		std::getline(iss, data);
		settings.modelScale = std::stof(data);
	}
	else {
		std::cout << "Error reading settings file. Using default settings." << std::endl;
	}

	std::cout << "Simulation Settings initilisatised with the following properties: " << std::endl;
	std::cout << "Num Agents: "			<< settings.numAgents << std::endl;
	std::cout << "Max Bound: "			<< settings.maxBound << std::endl;
	std::cout << "Max Velocity: "		<< settings.maxVelocity << std::endl;
	std::cout << "Max Steering Angle: " << settings.maxSteeringAngle << std::endl;
	std::cout << "Alignment Radius: "	<< settings.alignmentRadius << std::endl;
	std::cout << "Separation Radius: "	<< settings.separationRadius << std::endl;
	std::cout << "Cohesion Radius: "	<< settings.cohesionRadius << std::endl;
	std::cout << "Avoidance Radius: "	<< settings.avoidanceRadius << std::endl;
	std::cout << "Alignment Weight: "	<< settings.alignmentWeight << std::endl;
	std::cout << "Separation Weight: "	<< settings.separationWeight << std::endl;
	std::cout << "Cohesion Weight: "	<< settings.cohesionWeight << std::endl;
	std::cout << "Avoidance Weight: "	<< settings.avoidanceWeight << std::endl;
	std::cout << "Model Scale: "		<< settings.modelScale << std::endl;

	return settings;
}
