#pragma once
#include "../Common.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlsave.h>
	
namespace xml
{
	string XReadString(xmlNode* node, const string& attribute, const string& d);
	void SetAttribute(xmlNode* node, const string& attribute, const string& value);
	bool SaveFile(xmlDoc* doc, const string& path);
	xmlDoc* LoadFile(const string& path);

	template< typename type >
	type ReadAttribute (xmlNode * node, const string & attribute, const type & d)
	{
		xmlChar * data = xmlGetProp (node, (const xmlChar *) attribute.c_str());
		if (data != NULL)
		{
			type value;
			stringstream stream ((const char *) data);
			stream >> value;
				
			xmlFree (data);
				
			return value;
		}
			
		return d;
	}

	template<typename T>
	void SetAttribute(xmlNode * node, const string & attribute, const T & value)
	{
		stringstream ss;
		ss << value;
		SetAttribute(node, attribute, ss.str());
	}

	template<typename T>
	void SetOrUpdateChildValue(xmlNode* parent, const char* tagName, const T & value)
	{
		stringstream ss;
		ss << value;
		if (!parent || !tagName)
			return;

		// Search for existing child node with this name
		for (xmlNode* child = parent->children; child; child = child->next)
		{
			if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST tagName))
			{
				// Found existing — just update value
				SetAttribute(child, "value", ss.str());
				return;
			}
		}

		// Not found — create new node
		xmlNode* newNode = xmlNewChild(parent, NULL, BAD_CAST tagName, NULL);
		SetAttribute(newNode, "value", ss.str());
	}
}
