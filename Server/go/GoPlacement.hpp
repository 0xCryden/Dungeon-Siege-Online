#ifndef GOPLACEMENT_H_
#define GOPLACEMENT_H_

#include "GoComponent.hpp"
#include "../helper/SiegePos.h"
#include "../helper/vector_3.h"

class Region;
	
class GoPlacement : public GoComponent
{
	public:
			
		GoPlacement (Go * go);
		GoPlacement (Go * go, xmlNode * node);
			
		void Save (xmlNode* placementNode) const;

		bool IsDirty () const;
		void MarkAsClean ();
			
		string GetRegion () const;
		void SetRegion (const string & region);
			
		SiegePos Position () const;
		void SetPosition (const SiegePos & position);
			
		vector_3 WorldPosition () const;
		void SetWorldPosition (vector_3 worldPosition);
		void SetWorldPosition (float x, float y, float z);

	private:
			
		bool m_dirty;
		string m_region;
		SiegePos m_position;
		vector_3 m_worldPosition;

		uint8_t lastRotation;
		vector_3 lastLocal;
		SiegePos lastSiegePos;
};

#endif /* GOPLACEMENT_HPP_ */
