#pragma once

#include "../Common.h"

#include "GoComponent.hpp"

#include "../Gas/Gas.hpp"


class GoConversation : public GoComponent
{
public:
	GoConversation(Go* go);
	GoConversation(Go* go, const TemplateComponent* tmplComp);

private:
};