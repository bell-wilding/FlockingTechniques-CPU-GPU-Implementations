#pragma once

#include "../Common/Vector3.h"

namespace NCL {
	struct Agent {
		Vector3 position;
		Vector3 velocity;
		int cell;
	};
}