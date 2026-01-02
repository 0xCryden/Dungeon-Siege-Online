#pragma once

#include "../Common.h"

#include "GoComponent.hpp"

#include "../Gas/Gas.hpp"


class GoConversation : public GoComponent
{
public:
	GoConversation(Go* go);
	GoConversation(Go* go, const TemplateComponent* tmplComp);

	vector<string> Conversations() { return m_conversations; }
	void SetConversations(vector<string> conv) { m_conversations = conv; }

private:
	vector<string> m_conversations;
};