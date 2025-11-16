#include "Xml.h"

namespace xml
{
	string XReadString(xmlNode* node, const string& attribute, const string& d)
	{
		xmlChar* data = xmlGetProp(node, (const xmlChar*)attribute.c_str());
		if (data != NULL)
		{
			string value = (const char*)data;

			xmlFree(data);

			return value;
		}

		return d;
	}

	// --- Write functionality ---
	// Set or update attribute
	void SetAttribute(xmlNode* node, const string& attribute, const string& value)
	{
		xmlSetProp(node, (const xmlChar*)attribute.c_str(), (const xmlChar*)value.c_str());
	}

	// Save modified document to file
	bool SaveFile(xmlDoc* doc, const string& path)
	{
		int res = xmlSaveFormatFileEnc(path.c_str(), doc, "UTF-8", 1);
		return res != -1;
	}

	// Load XML from file
	xmlDoc* LoadFile(const string& path)
	{
		return xmlReadFile(path.c_str(), NULL, 0);
	}
}
