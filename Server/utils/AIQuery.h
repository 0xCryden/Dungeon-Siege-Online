#pragma once
#include "../map/WorldMap.h"
	
class AIQuery
{
	public:
		static bool IsInRange (const Go * a, const Go * b, double range);
};
