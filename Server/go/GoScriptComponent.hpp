#ifndef GOSCRIPTCOMPONENT_HPP_
#define GOSCRIPTCOMPONENT_HPP_

#include "GoComponent.hpp"
#include "../msg/WorldMessage.h"
	
class GoScriptComponent : public GoComponent
{
	public:
			
		GoScriptComponent (Go * go);
			
		virtual ~GoScriptComponent () {};
			
		virtual string Name () const = 0;
			
		virtual void OnGoHandleMessage (const WorldMessage & message) = 0;
};

#endif /* GOSCRIPTCOMPONENT_HPP_ */
