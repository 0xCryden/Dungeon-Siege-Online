#pragma once
#include "../platform/Platform.h"

struct SiegePos
{
	SiegePos () : Node (0), X (0.0f), Y (0.0f), Z (0.0f) {}
	SiegePos (uint32_t id, float x, float y, float z) : Node (id), X (x), Y (y), Z (z) {}
		
	bool IsValid () const {return Node != 0x00000000;}

	uint32_t Node;
	float X;
	float Y;
	float Z;
};