#include "GoScriptComponent.hpp"

GoScriptComponent :: GoScriptComponent (Go * go) : GoComponent (go)
{
}

/*
int GoScriptComponent :: GetComponentInt (const string & property)
{
	map<string, int>::iterator iterator = m_integers.find (property);
	return iterator != m_integers.end() ? iterator->second : 0;
}

float GoScriptComponent :: GetComponentFloat (const string & property)
{
	map<string, float>::iterator iterator = m_floats.find (property);
	return iterator != m_floats.end() ? iterator->second : 0.0f;
}

bool GoScriptComponent :: GetComponentBool (const string & property)
{
	map<string, bool>::iterator iterator = m_booleans.find (property);
	return iterator != m_booleans.end() ? iterator->second : false;
}

string GoScriptComponent :: GetComponentString (const string & property)
{
	map<string, string>::iterator iterator = m_strings.find (property);
	return iterator != m_strings.end() ? iterator->second : "";
}

void GoScriptComponent :: SetComponentInt (const string & property, int value)
{
	m_integers[property] = value;
}

void GoScriptComponent :: SetComponentFloat (const string & property, float value)
{
	m_floats[property] = value;
}

void GoScriptComponent :: SetComponentBool (const string & property, bool value)
{
	m_booleans[property] = value;
}

void GoScriptComponent :: SetComponentString (const string & property, const string & value)
{
	m_strings[property] = value;
}
*/
