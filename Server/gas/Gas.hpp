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
	std::unordered_map<std::string, std::string> fields;
	std::unordered_map<std::string, TemplateComponent> subcomponents;
	// Get a field value by key
	/*std::optional<std::string> GetField(const std::string& key) const {
		auto it = fields.find(key);
		return it != fields.end() ? std::optional(it->second) : std::nullopt;
	}*/

	const std::string* GetField(const std::string& key) const {
		auto it = fields.find(key);
		return it != fields.end() ? &it->second : nullptr;
	}
	int GetInt(const std::string& key, int defaultValue = 0) const {
		if (auto* str = GetField(key))
			return std::stoi(*str);
		return defaultValue;
	}
	float GetFloat(const std::string& key, float defaultValue = 0.0f) const {
		if (auto* str = GetField(key))
			return std::stof(*str);
		return defaultValue;
	}    
	
	// Extract the part before the comma (level)
	std::string SkillLevelString(const std::string& key) const
	{
		if (const std::string* val = GetField(key))
		{
			size_t commaPos = val->find(',');
			if (commaPos != std::string::npos)
				return val->substr(0, commaPos);
			else
				return *val; // whole string if no comma
		}
		return ""; // field not found
	}
	// Extract the part after the comma (experience)
	std::string SkillExpString(const std::string& key) const
	{
		if (const std::string* val = GetField(key))
		{
			size_t commaPos = val->find(',');
			if (commaPos != std::string::npos)
				return val->substr(commaPos + 1);
			else
				return "0"; // default 0 if no comma
		}
		return "0"; // field not found
	}

	// Get a nested component by name
	TemplateComponent* GetSubcomponent(const std::string& name) {
		auto it = subcomponents.find(name);
		return it != subcomponents.end() ? &it->second : nullptr;
	}

	const TemplateComponent* GetSubcomponent(const std::string& name) const {
		auto it = subcomponents.find(name);
		return it != subcomponents.end() ? &it->second : nullptr;
	}

	std::vector<std::string> GetFieldKeys() const {
		std::vector<std::string> keys;
		for (auto& pair : fields)
			keys.push_back(pair.first);
		return keys;
	}

	std::vector<std::string> GetSubcomponentNames() const {
		std::vector<std::string> names;
		for (auto& pair : subcomponents)
			names.push_back(pair.first);
		return names;
	}
};

struct TemplateData
{
	std::string name;
	std::string doc;
	std::string specializes;
	std::string region;
	std::string scid;
	std::unordered_map<std::string, TemplateComponent> components;
	// Get pointer to a component by name
	TemplateComponent* GetComponent(const std::string& name) {
		auto it = components.find(name);
		return it != components.end() ? &it->second : nullptr;
	}

	const TemplateComponent* GetComponent(const std::string& name) const {
		auto it = components.find(name);
		return it != components.end() ? &it->second : nullptr;
	}
};

class TemplateManager {
public:
	// Store all templates by name
	std::unordered_map<std::string, TemplateData> templates;
	// Store all map templates by scid
	std::unordered_map<std::string, TemplateData> mapTemplates;

	// Load a template into memory
	void AddTemplate(TemplateData tmpl) {
		templates[tmpl.name] = std::move(tmpl);
	}

	void AddMapTemplate(TemplateData tmpl) {
		mapTemplates[tmpl.scid] = std::move(tmpl);
	}

	// Get template by name
	TemplateData* GetTemplate(const std::string& name) {
		auto it = templates.find(name);
		return it != templates.end() ? &it->second : nullptr;
	}

	const TemplateData* GetTemplate(const std::string& name) const {
		auto it = templates.find(name);
		return it != templates.end() ? &it->second : nullptr;
	}

	TemplateData* GetMapTemplate(const std::string& scid) {
		auto it = mapTemplates.find(scid);
		return it != mapTemplates.end() ? &it->second : nullptr;
	}

	const TemplateData* GetMapTemplate(const std::string& scid) const {
		auto it = mapTemplates.find(scid);
		return it != mapTemplates.end() ? &it->second : nullptr;
	}

	void MergeComponent(TemplateComponent& target, const TemplateComponent& parent);
	void ResolveTemplateInheritance(); 
	void MergeTemplates(TemplateData& child, const TemplateData& parent);

	const auto& GetAll() const { return templates; }
	const auto& GetAllMap() const { return mapTemplates; }
};

extern TemplateManager manager;

struct PlacementData {
	std::string templateName;
	std::string instanceName;

	SiegePos position;
	vector_3 orientation; // Store only the vector part of the quaternion
	std::string regionName; 
	vector<string> conversations; 
	unordered_map<string, TemplateComponent> components;
};

class PlacementManager {
private:
	std::unordered_map<std::string, PlacementData> byInstanceName;
	std::unordered_multimap<std::string, PlacementData*> byTemplateName;

public:
	void AddPlacement(PlacementData data) {
		std::string key = data.instanceName;
		auto inserted = byInstanceName.emplace(key, std::move(data));
		if (inserted.second) {
			byTemplateName.emplace(inserted.first->second.templateName, &inserted.first->second);
		}
	}

	const PlacementData* GetByInstanceName(const std::string& instanceName) const {
		auto it = byInstanceName.find(instanceName);
		return it != byInstanceName.end() ? &it->second : nullptr;
	}

	std::vector<const PlacementData*> GetByTemplateName(const std::string& templateName) const {
		std::vector<const PlacementData*> result;
		auto range = byTemplateName.equal_range(templateName);
		for (auto it = range.first; it != range.second; ++it) {
			result.push_back(it->second);
		}
		return result;
	}

	std::vector<const PlacementData*> GetAllPlacements() const {
		std::vector<const PlacementData*> result;
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
	void LoadGasToGo();

	// Helper functions
	std::string StripLineComment(const std::string& line);
	std::string Trim(const std::string& s);
	std::string ParseComponentName(const std::string& raw);
	bool StartsWith(const std::string& str, const std::string& prefix);
	void ParseGasBlock(std::istream& stream, TemplateComponent& outComp);
	bool ReadActorPlacements(const std::string& fullPath, std::vector<PlacementData>& outPlacements);
	bool ReadTemplatesFile(const std::string& fullPath, std::unordered_map<std::string, TemplateData>& outTemplates,
		const std::unordered_set<std::string>& allowedComponents = {});
	bool ReadMapTemplatesFile(const std::string& fullPath, std::unordered_map<std::string, TemplateData>& outTemplates,
		const std::unordered_set<std::string>& allowedComponents = {});
	void LogComponent(const std::string& name, const TemplateComponent& comp, const std::string& indent = "");
private:
};
extern Gas gas;

#endif /* GAS_H_ */
