#include "Node.h"

Node :: Node (uint32_t id, float x, float y, float z, uint8_t rotation) : m_id (id), m_x (x), m_y (y), m_z (z), m_rotation (rotation)
{
}

uint32_t Node :: Id () const
{
	return m_id;
}

float Node :: X () const
{
	return m_x;
}

float Node :: Y () const
{
	return m_y;
}

float Node :: Z () const
{
	return m_z;
}

uint8_t Node :: Rotation () const
{
	return m_rotation;
}
