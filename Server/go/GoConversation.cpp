
#include "Go.hpp"
#include "GoConversation.h"
#include "../Engine.hpp"
#include "../helper/Helper.h"


GoConversation::GoConversation(Go* go) : GoComponent(go)
{
}

GoConversation::GoConversation(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
}