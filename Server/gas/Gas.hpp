/*
 * Gas.hpp
 *
 *  Created on: 29.05.2025
 *      Author: Cryden
 */
#ifndef GAS_H_
#define GAS_H_

#include "../Common.h"
#include "../helper/SiegePos.h"
#include "../helper/vector_3.h"
#include <string>
#include <unordered_set>
#include <iostream>
#include <functional>
#include <fstream>

struct TemplateComponent
{
	unordered_map<string, string> fields;
	unordered_map<string, TemplateComponent> subcomponents;
	// Get a field value by key
	/*optional<string> GetField(const string& key) const {
		auto it = fields.find(key);
		return it != fields.end() ? optional(it->second) : nullopt;
	}*/

	const string* GetField(const string& key) const {
		auto it = fields.find(key);
		return it != fields.end() ? &it->second : nullptr;
	}
	int GetInt(const string& key, int defaultValue = 0) const {
		if (auto* str = GetField(key))
			return stoi(*str);
		return defaultValue;
	}
	float GetFloat(const string& key, float defaultValue = 0.0f) const {
		if (auto* str = GetField(key))
			return stof(*str);
		return defaultValue;
	}    
	
	// Extract the part before the comma (level)
	string SkillLevelString(const string& key) const
	{
		if (const string* val = GetField(key))
		{
			size_t commaPos = val->find(',');
			if (commaPos != string::npos)
				return val->substr(0, commaPos);
			else
				return *val; // whole string if no comma
		}
		return ""; // field not found
	}
	// Extract the part after the comma (experience)
	string SkillExpString(const string& key) const
	{
		if (const string* val = GetField(key))
		{
			size_t commaPos = val->find(',');
			if (commaPos != string::npos)
				return val->substr(commaPos + 1);
			else
				return "0"; // default 0 if no comma
		}
		return "0"; // field not found
	}

	// Get a nested component by name
	TemplateComponent* GetSubcomponent(const string& name) {
		auto it = subcomponents.find(name);
		return it != subcomponents.end() ? &it->second : nullptr;
	}

	const TemplateComponent* GetSubcomponent(const string& name) const {
		auto it = subcomponents.find(name);
		return it != subcomponents.end() ? &it->second : nullptr;
	}

	vector<string> GetFieldKeys() const {
		vector<string> keys;
		for (auto& pair : fields)
			keys.push_back(pair.first);
		return keys;
	}

	vector<string> GetSubcomponentNames() const {
		vector<string> names;
		for (auto& pair : subcomponents)
			names.push_back(pair.first);
		return names;
	}
};

struct TemplateData
{
	string name;
	string doc;
	string specializes;
	string region;
	string scid;
	unordered_map<string, TemplateComponent> components;
	// Get pointer to a component by name
	TemplateComponent* GetComponent(const string& name) {
		auto it = components.find(name);
		return it != components.end() ? &it->second : nullptr;
	}

	const TemplateComponent* GetComponent(const string& name) const {
		auto it = components.find(name);
		return it != components.end() ? &it->second : nullptr;
	}
};

class TemplateManager {
public:
	// Store all templates by name
	unordered_map<string, TemplateData> templates;
	// Store all map templates by scid
	unordered_map<string, TemplateData> mapTemplates;

	// Load a template into memory
	void AddTemplate(TemplateData tmpl) {
		templates[tmpl.name] = move(tmpl);
	}

	void AddMapTemplate(TemplateData tmpl) {
		mapTemplates[tmpl.scid] = move(tmpl);
	}

	// Get template by name
	TemplateData* GetTemplate(const string& name) {
		auto it = templates.find(name);
		return it != templates.end() ? &it->second : nullptr;
	}

	const TemplateData* GetTemplate(const string& name) const {
		auto it = templates.find(name);
		return it != templates.end() ? &it->second : nullptr;
	}

	TemplateData* GetMapTemplate(const string& scid) {
		auto it = mapTemplates.find(scid);
		return it != mapTemplates.end() ? &it->second : nullptr;
	}

	const TemplateData* GetMapTemplate(const string& scid) const {
		auto it = mapTemplates.find(scid);
		return it != mapTemplates.end() ? &it->second : nullptr;
	}

	void MergeComponent(TemplateComponent& target, const TemplateComponent& parent);
	void ResolveTemplateInheritance();
	void MergeTemplates(TemplateData& child, const string& parent);
	void MergeTemplates(TemplateData& child, const TemplateData& parent);

	auto& GetAll() { return templates; }
	const auto& GetAll() const { return templates; }
	auto& GetAllMap() { return mapTemplates; }
	const auto& GetAllMap() const { return mapTemplates; }
};

extern TemplateManager manager;

struct PlacementData {
	string templateName;
	string instanceName;

	SiegePos position;
	vector_3 orientation; // Store only the vector part of the quaternion
	string regionName; 
	vector<string> conversations; 
	unordered_map<string, TemplateComponent> components;
};

class PlacementManager {
private:
	unordered_map<string, PlacementData> byInstanceName;
	unordered_multimap<string, PlacementData*> byTemplateName;

public:
	void AddPlacement(PlacementData data) {
		string key = data.instanceName;
		auto inserted = byInstanceName.emplace(key, move(data));
		if (inserted.second) {
			byTemplateName.emplace(inserted.first->second.templateName, &inserted.first->second);
		}
	}

	const PlacementData* GetByInstanceName(const string& instanceName) const {
		auto it = byInstanceName.find(instanceName);
		return it != byInstanceName.end() ? &it->second : nullptr;
	}

	vector<const PlacementData*> GetByTemplateName(const string& templateName) const {
		vector<const PlacementData*> result;
		auto range = byTemplateName.equal_range(templateName);
		for (auto it = range.first; it != range.second; ++it) {
			result.push_back(it->second);
		}
		return result;
	}

	vector<const PlacementData*> GetAllPlacements() const {
		vector<const PlacementData*> result;
		result.reserve(byInstanceName.size());
		for (const auto& [_, placement] : byInstanceName) {
			result.push_back(&placement);
		}
		return result;
	}

	const auto& GetAll() const { return byInstanceName; }
};
extern PlacementManager placementManager;

class Gas {
public:
	Gas();
	~Gas();

	void LoadTemplates();
	void LoadMapTemplates();

	// Helper functions
	string StripLineComment(const string& line);
	string Trim(const string& s);
	string ParseComponentName(const string& raw);
	bool StartsWith(const string& str, const string& prefix);
	void ParseGasBlock(istream& stream, TemplateComponent& outComp);
	bool ReadActorPlacements(const string& fullPath, vector<PlacementData>& outPlacements);
	bool ReadTemplatesFile(const string& fullPath, unordered_map<string, TemplateData>& outTemplates,
		const unordered_set<string>& allowedComponents = {});
	bool ReadMapTemplatesFile(const string& fullPath, unordered_map<string, TemplateData>& outTemplates,
		const unordered_set<string>& allowedComponents = {});
	void LogComponent(const string& name, const TemplateComponent& comp, const string& indent = "");
private:
};
extern Gas gas;

#endif /* GAS_H_ */
