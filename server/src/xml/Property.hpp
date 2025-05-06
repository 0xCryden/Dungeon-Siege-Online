#ifndef PROPERTY_HPP_
#define PROPERTY_HPP_

	#include <libxml/tree.h>
	#include <libxml/parser.h>
	#include <libxml/xmlsave.h>
	#include "../platform.hpp"
	
	#include <string>
	using std::string;
	
	#include <sstream>
	using std::stringstream;

	#include <iostream>
	
	namespace xml
	{
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
		
		inline string XReadString (xmlNode * node, const string & attribute, const string & d)
		{
			xmlChar * data = xmlGetProp (node, (const xmlChar *) attribute.c_str());
			if (data != NULL)
			{
				string value = (const char *) data;
				
				xmlFree (data);
				
				return value;
			}
			
			return d;
		}

		// --- Write functionality ---

		// Set or update attribute
		inline void SetAttribute(xmlNode * node, const string & attribute, const string & value)
		{
			xmlSetProp(node, (const xmlChar *)attribute.c_str(), (const xmlChar *)value.c_str());
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

		// Remove a node by <go id="x">
		/*inline void RemoveGO(xmlDoc * doc, const string & id)
		{
			xmlNode * root = xmlDocGetRootElement(doc);
			for (xmlNode * node = root->children; node; node = node->next)
			{
				if (node->type == XML_ELEMENT_NODE && xmlStrEqual(node->name, BAD_CAST "go"))
				{
					xmlChar * prop = xmlGetProp(node, BAD_CAST "id");
					if (prop && id == (const char *)prop)
					{
						xmlUnlinkNode(node);
						xmlFreeNode(node);
						xmlFree(prop);
						break;
					}
					if (prop) xmlFree(prop);
				}
			}
		}

		// Add a new <go> node with given id
		inline xmlNode * AddGO(xmlDoc * doc, const string & id)
		{
			xmlNode * root = xmlDocGetRootElement(doc);
			xmlNode * go = xmlNewChild(root, NULL, BAD_CAST "go", NULL);
			SetAttribute(go, "id", id);
			return go;
		}*/

		// Save modified document to file
		inline bool SaveFile(xmlDoc * doc, const string & path)
		{
			int res = xmlSaveFormatFileEnc(path.c_str(), doc, "UTF-8", 1);
			return res != -1;
		}

		// Load XML from file
		inline xmlDoc * LoadFile(const string & path)
		{
			return xmlReadFile(path.c_str(), NULL, 0);
		}

	}

#endif /* PROPERTY_HPP_ */
