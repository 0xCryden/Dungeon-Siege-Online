#pragma once
#include "../helper/Log.h"
#include "../platform/Platform.h"
	
class Node
{
	public:
		Node (uint32_t id, float x, float y, float z, uint8_t rotation);
			
		uint32_t Id () const;			
		float X () const;
		float Y () const;
		float Z () const;
		uint8_t Rotation () const;
			
	private:
		uint32_t m_id;
		float m_x;
		float m_y;
		float m_z;
		uint8_t m_rotation;
};
