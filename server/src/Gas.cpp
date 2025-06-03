/*
 * Gas.cpp
 *
 *  Created on: 29.05.2025
 *      Author: Cryden
 */

#include "Gas.hpp"
#include <algorithm>
#include <functional>

using namespace std;
namespace fs = std::filesystem;

Gas gas;
PlacementManager placementManager;
TemplateManager manager;
// Inside TemplateManager.cpp
void TemplateManager::MergeComponent(TemplateComponent& target, const TemplateComponent& parent) {
    // Merge fields
    for (const auto& [key, val] : parent.fields) {
        if (target.fields.find(key) == target.fields.end()) {
            target.fields[key] = val;
        }
    }

    // Merge subcomponents
    for (const auto& [subName, subComp] : parent.subcomponents) {
        auto& targetSub = target.subcomponents[subName];
        MergeComponent(targetSub, subComp);  // recursive merge
    }
}

void TemplateManager::ResolveTemplateInheritance() {
    std::unordered_set<std::string> resolving;

    size_t totalLoaded = 0;
    std::function<void(TemplateData&)> resolve = [&](TemplateData& tpl) {
        if (tpl.specializes.empty())
            return;

        if (resolving.count(tpl.name)) {
            std::cerr << "[ERROR] Circular specialization detected at: " << tpl.name << std::endl;
            return;
        }

        auto it = templates.find(tpl.specializes);  // <- Use internal templates map
        if (it == templates.end()) {
            std::cerr << "[ERROR] Missing parent template: " << tpl.specializes << " for " << tpl.name << std::endl;
            return;
        }

        resolving.insert(tpl.name);
        resolve(it->second);  // Recursively resolve parent first

        for (auto& [compName, parentComp] : it->second.components) {
            //std::cout << "Merging component: " << compName << " from " << it->first << " into " << tpl.name << std::endl;
            auto& derivedComp = tpl.components[compName];
            MergeComponent(derivedComp, parentComp);
            ++totalLoaded;
        }

        resolving.erase(tpl.name);
    };

    for (auto& [name, tpl] : templates) {
        resolve(tpl);
    }
    std::cout << "[INFO] Finished resolving Inheritances. Total components merged: " << totalLoaded << std::endl;
}

Gas :: Gas()
{
}

Gas :: ~Gas ()
{
}

std::string Gas :: StripLineComment(const std::string& line) {
    size_t pos = line.find("//");
    return pos != std::string::npos ? line.substr(0, pos) : line;
}
// --- Trim utility ---
string Gas :: Trim(const string& s) {
	auto start = s.find_first_not_of(" \t\r\n");
	auto end = s.find_last_not_of(" \t\r\n");
	if (start == string::npos || end == string::npos) return "";
	return s.substr(start, end - start + 1);
}

// --- Extracts component name from "[name]" or "[name*]" ---
string Gas :: ParseComponentName(const string& raw)
{
    // Find first [ and ] and extract substring inside
    size_t start = raw.find('[');
    size_t end = raw.find(']', start);
    if (start == string::npos || end == string::npos || end <= start)
        return ""; // Invalid format, or no brackets found

    string trimmed = raw.substr(start + 1, end - start - 1);

    // Remove trailing '*' if present (wildcard)
    if (!trimmed.empty() && trimmed.back() == '*')
        trimmed.pop_back();

    // Handle nested names: keep only up to first ':'
    size_t colon = trimmed.find(':');
    if (colon != string::npos)
        trimmed = trimmed.substr(0, colon);

    return Trim(trimmed);
}

bool Gas :: StartsWith(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.size(), prefix) == 0;
}

void Gas :: ParseGasBlock(istream& stream, TemplateComponent& outComp)
{
    string line;
    vector<TemplateComponent*> stack;
    stack.push_back(&outComp);

    string pendingComponentName;
    bool inBlockComment = false;

    while (getline(stream, line))
    {
    	if (inBlockComment) {
			size_t endPos = line.find("*/");
			if (endPos != std::string::npos) {
				inBlockComment = false;
				line = line.substr(endPos + 2); // Skip past the block comment end
			} else {
				continue; // Still inside block comment — skip line
			}
		}

		// Remove any block comment opening
		size_t blockStart = line.find("/*");
		if (blockStart != std::string::npos) {
			size_t blockEnd = line.find("*/", blockStart + 2);
			if (blockEnd != std::string::npos) {
				// Inline block comment: remove it
				line.erase(blockStart, blockEnd - blockStart + 2);
			} else {
				// Block comment starts but doesn’t end yet — strip rest and flag
				line = line.substr(0, blockStart);
				inBlockComment = true;
			}
		}

		// Remove // line comments
		line = StripLineComment(line);

		line = Trim(line);
		if (line.empty()) continue;

        // End of current block
        if (line == "}")
        {
            if (!stack.empty()) stack.pop_back();
            if (stack.empty()) break;
            continue;
        }

        // Start of new component — either "[name]" alone or "[name] {"
        if (StartsWith(line, "[") && line.find(']') != string::npos)
        {
            // Extract component name between [ and ]
            pendingComponentName = ParseComponentName(line);

            // Find '{' and '}' on the same line, if any
            size_t braceOpen = line.find('{');
            size_t braceClose = line.find('}');

            TemplateComponent newComp;
            TemplateComponent* parent = stack.back();
            auto& inserted = parent->subcomponents[pendingComponentName];
            inserted = std::move(newComp);
            stack.push_back(&inserted);

            if (braceOpen != string::npos && braceClose != string::npos && braceClose > braceOpen)
            {
                // Extract the content inside { ... }
                std::string insideBraces = line.substr(braceOpen + 1, braceClose - braceOpen - 1);
                insideBraces = Trim(insideBraces);

                // Now parse fields insideBraces separated by ';'
                size_t start = 0;
                while (start < insideBraces.length())
                {
                    size_t end = insideBraces.find(';', start);
                    std::string field;
                    if (end == std::string::npos)
                    {
                        field = insideBraces.substr(start);
                        start = insideBraces.length();
                    }
                    else
                    {
                        field = insideBraces.substr(start, end - start);
                        start = end + 1;
                    }
                    field = Trim(field);
                    if (!field.empty())
                    {
                        size_t eqPos = field.find('=');
                        if (eqPos != std::string::npos)
                        {
                            std::string key = Trim(field.substr(0, eqPos));
                            std::string val = Trim(field.substr(eqPos + 1));
                            inserted.fields[key] = val;
                        }
                    }
                }

                // Since block is closed on same line, pop the stack now
                if (stack.size() > 1)
                    stack.pop_back();
            }
            else if (braceOpen != string::npos && (braceClose == string::npos || braceClose < braceOpen))
            {
                // Opening brace with no closing brace on the same line — block continues, keep stack
                // pendingComponentName cleared because already processed
                pendingComponentName.clear();
            }
            else
            {
                // No brace on this line, just a component name, wait for '{' on next lines
                pendingComponentName.clear();
            }
            continue;
        }
        /*
        if (StartsWith(line, "[") && line.find(']') != string::npos)
		{
			// Extract component name between [ and ]
			pendingComponentName = ParseComponentName(line);

			// Check for inline block { ... }
			size_t braceOpen = line.find('{');
			size_t braceClose = line.find('}');

			if (braceOpen != string::npos && braceClose != string::npos && braceClose > braceOpen)
			{
				// Inline block present on same line, create component now
				TemplateComponent newComp;
				TemplateComponent* parent = stack.back();
				auto& inserted = parent->subcomponents[pendingComponentName];
				inserted = std::move(newComp);
				stack.push_back(&inserted);

				// Parse fields inside { ... }
				std::string insideBraces = line.substr(braceOpen + 1, braceClose - braceOpen - 1);
				insideBraces = Trim(insideBraces);

				size_t start = 0;
				while (start < insideBraces.length())
				{
					size_t end = insideBraces.find(';', start);
					std::string field;
					if (end == std::string::npos)
					{
						field = insideBraces.substr(start);
						start = insideBraces.length();
					}
					else
					{
						field = insideBraces.substr(start, end - start);
						start = end + 1;
					}
					field = Trim(field);
					if (!field.empty())
					{
						size_t eqPos = field.find('=');
						if (eqPos != std::string::npos)
						{
							std::string key = Trim(field.substr(0, eqPos));
							std::string val = Trim(field.substr(eqPos + 1));
							inserted.fields[key] = val;
						}
					}
				}

				// Inline block closes immediately, pop stack now
				if (stack.size() > 1)
					stack.pop_back();

				pendingComponentName.clear();
			}
			else
			{
				// No inline block; wait for '{' line before creating component
				// Keep pendingComponentName, but do not insert yet
			}
			continue;
		}
        */
        // Block opening after a standalone [name] on previous line
        if (line == "{" && !pendingComponentName.empty())
        {
            TemplateComponent newComp;
            TemplateComponent* parent = stack.back();
            auto& inserted = parent->subcomponents[pendingComponentName];
            inserted = std::move(newComp);
            stack.push_back(&inserted);
            pendingComponentName.clear();
            continue;
        }

        // Field assignment
        size_t eq = line.find('=');
        if (eq != string::npos)
        {
            string key = Trim(line.substr(0, eq));
            string val = Trim(line.substr(eq + 1));
            if (!val.empty() && val.back() == ';')
                val.pop_back();

            stack.back()->fields[key] = val;
        }
    }
}

bool Gas :: ReadActorPlacements(const std::string& fullPath, std::vector<PlacementData>& outPlacements)
{
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open actor placement file: " << fullPath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = Trim(line);
        if (line.rfind("[t:", 0) == 0 && line.find(",n:") != std::string::npos) {
            PlacementData data;
            data.orientation = { 0.0f, 0.0f, 0.0f };
            data.position = SiegePos(); // default init

            // Parse t: and n:
            size_t tPos = line.find("t:");
            size_t nPos = line.find("n:");
            size_t end = line.find(']');

            if (tPos == std::string::npos || nPos == std::string::npos || end == std::string::npos) {
                std::cerr << "[WARN] Malformed header: " << line << std::endl;
                continue;
            }

            data.templateName = Trim(line.substr(tPos + 2, nPos - tPos - 3));
            data.instanceName = Trim(line.substr(nPos + 2, end - nPos - 2));

            // Read full actor block (handle nested braces)
            std::stringstream actorBlock;
            int braceDepth = 0;
            char ch;

            // First opening brace
            while (file.get(ch)) {
                if (ch == '{') {
                    braceDepth = 1;
                    actorBlock << ch;
                    break;
                }
            }

            while (braceDepth > 0 && file.get(ch)) {
                actorBlock << ch;
                if (ch == '{') braceDepth++;
                else if (ch == '}') braceDepth--;
            }

            // Search for [placement] block inside actorBlock
            std::string blockLine;
            bool inPlacement = false;
            int placementDepth = 0;

            while (std::getline(actorBlock, blockLine)) {
                blockLine = Trim(blockLine);
                if (blockLine.empty()) continue;

                if (!inPlacement && blockLine == "[placement]") {
                    // Enter placement block
                    while (actorBlock.get(ch)) {
                        if (ch == '{') {
                            inPlacement = true;
                            placementDepth = 1;
                            break;
                        }
                    }
                    continue;
                }

                if (inPlacement) {
                    if (blockLine.find('}') != std::string::npos) {
                        placementDepth--;
                        if (placementDepth <= 0) break;
                        continue;
                    }

                    size_t eq = blockLine.find('=');
                    if (eq == std::string::npos) continue;

                    std::string key = Trim(blockLine.substr(0, eq));
                    std::string value = Trim(blockLine.substr(eq + 1));

                    if (key == "q orientation") {
                        float ox, oy, oz, ow;
                        if (sscanf(value.c_str(), "%f,%f,%f,%f", &ox, &oy, &oz, &ow) == 4) {
                        	data.orientation = { oy, oz, ow }; // ignore x
                            //data.orientation = { ox, oy, oz }; // ignore w
                        }
                    }
                    else if (key == "p position") {
                        float px, py, pz;
                        uint32_t node;
                        if (sscanf(value.c_str(), "%f,%f,%f,0x%x", &px, &py, &pz, &node) == 4) {
                            data.position = SiegePos(node, px, py, pz);
                        }
                    }
                }
            }

            outPlacements.push_back(std::move(data));
        }
    }

    return !outPlacements.empty();
}

bool Gas :: ReadTemplatesFile(const string& fullPath, unordered_map<string, TemplateData>& outTemplates, const std::unordered_set<string>& allowedComponents)
{
    //cerr << "[INFO] Reading templates from: " << fullPath << endl;

    ifstream file(fullPath);
    if (!file.is_open()) {
        cerr << "[ERROR] Failed to open file: " << fullPath << endl;
        return false;
    }

    string line;
    while (getline(file, line))
    {
        line = Trim(line);
        if (line.rfind("[t:template,", 0) == 0)
        {
            TemplateData tpl;

            size_t name_pos = line.find("n:");
            if (name_pos == string::npos) {
                cerr << "[ERROR] Template line missing 'n:' in: " << line << endl;
                continue;
            }

            name_pos += 2;
            size_t name_end = line.find(']', name_pos);
            if (name_end == string::npos) {
                cerr << "[ERROR] Failed to find closing ']' after template name in: " << line << endl;
                continue;
            }

            tpl.name = Trim(line.substr(name_pos, name_end - name_pos));
            std::transform(tpl.name.begin(), tpl.name.end(), tpl.name.begin(), ::tolower);
            //cerr << "[INFO] Found template: " << tpl.name << endl;

            // Skip until '{'
            char ch;
            bool foundBlockStart = false;
            while (file.get(ch)) {
                if (ch == '{') {
                    foundBlockStart = true;
                    break;
                }
            }

            if (!foundBlockStart) {
                cerr << "[ERROR] Failed to find '{' for template: " << tpl.name << endl;
                continue;
            }

            // Parse template block
            TemplateComponent root;
            try {
                ParseGasBlock(file, root);
            } catch (const exception& e) {
                cerr << "[ERROR] Exception while parsing GAS block for template '" << tpl.name << "': " << e.what() << endl;
                continue;
            }

            tpl.components = move(root.subcomponents);

            // allow list
            /*if (!allowedComponents.empty()) {
                for (auto it = tpl.components.begin(); it != tpl.components.end(); ) {
                    if (allowedComponents.find(it->first) == allowedComponents.end()) {
                        it = tpl.components.erase(it);  // Remove component not in allowlist
                    } else {
                        ++it;
                    }
                }
            }*/

            if (root.fields.count("specializes")) {
                tpl.specializes = root.fields["specializes"];
                //cerr << "[INFO] Template '" << tpl.name << "' specializes: " << tpl.specializes << endl;
            }

            if (root.fields.count("doc")) {
                tpl.doc = root.fields["doc"];
            }

            outTemplates[tpl.name] = move(tpl);
           // cerr << "[INFO] Template '" << tpl.name << "' parsed successfully." << endl;
        }
    }

    if (outTemplates.empty()) {
        cerr << "[WARN] No templates found in file: " << fullPath << endl;
        return false;
    }

    //cerr << "[INFO] Successfully parsed " << outTemplates.size() << " templates from: " << fullPath << endl;
    return true;
}

void Gas :: LogComponent(const std::string& name, const TemplateComponent& comp, const std::string& indent)
{
    std::cout << indent << "- [" << name << "] (fields: " << comp.fields.size()
              << ", subcomponents: " << comp.subcomponents.size() << ")" << std::endl;

    for (const auto& [key, value] : comp.fields)
    {
        std::cout << indent << "    " << key << " = " << value << std::endl;
    }

    for (const auto& [subName, subComp] : comp.subcomponents)
    {
        LogComponent(subName, subComp, indent + "  ");
    }
}

void Gas :: LoadTemplates()
{
    std::unordered_map<std::string, TemplateData> templates;
    std::unordered_set<std::string> allowed = { "actor", "aspect", "mind" };

    const std::string rootPath = "data/static/templates";

    size_t totalLoaded = 0;

    for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".gas") {
            const std::string filePath = entry.path().string();

            std::unordered_map<std::string, TemplateData> fileTemplates;
            if (ReadTemplatesFile(filePath, fileTemplates, allowed)) {
                for (auto& [name, tpl] : fileTemplates) {
                    //std::cout << "Loaded Template: " << name << " from " << filePath << std::endl;
                    manager.AddTemplate(std::move(tpl));
                    ++totalLoaded;
                }
            }
        }
    }
    manager.ResolveTemplateInheritance();

    std::cout << "[INFO] Finished loading templates. Total loaded: " << totalLoaded << std::endl;
}

void Gas::LoadMapTemplates() {
    namespace fs = std::filesystem;

    std::string basePath = "data/static/map/multiplayer_world/regions";
    size_t totalLoaded = 0;

    for (const auto& entry : fs::recursive_directory_iterator(basePath)) {
        if (!entry.is_regular_file())
            continue;

        const fs::path& path = entry.path();

        // We only want .../objects/regular/actor.gas
        if (path.filename() == "actor.gas") {
            auto parent = path.parent_path();                 // regular
            auto grandparent = parent.parent_path();          // objects

            if (parent.filename() == "regular" && grandparent.filename() == "objects") {
                std::vector<PlacementData> placements;
                std::string fullPath = path.string();

                if (ReadActorPlacements(fullPath, placements)) {
                    //std::cout << "Loaded " << placements.size() << " actor placements from " << fullPath << ":\n";

                    for (auto& p : placements) {
                        /*std::cout << "Loaded: " << p.templateName << " at ("
                                  << p.position.X << ", " << p.position.Y << ", " << p.position.Z
                                  << "), facing (" << p.orientation.x << ", " << p.orientation.y << ", " << p.orientation.z << ")\n";*/

                        placementManager.AddPlacement(std::move(p));
                        ++totalLoaded;
                    }
                } else {
                    std::cerr << "Failed to load actor placements from " << fullPath << "\n";
                }
            }
        }
    }

    std::cout << "Total actor placements loaded: " << totalLoaded << "\n";
}

void Gas::LoadGasToGo()
{
	/*for (const auto& [instanceName, placement] : placementManager.GetAll())
	{
		const std::string& templateName = placement.templateName;

		// Skip if already instantiated
		if (m_contentdb.find(templateName) != m_contentdb.end())
			continue;

		if (templateName.empty())
			continue;

		try
		{
			const TemplateData* tmpl = manager.GetTemplate(templateName);
			if (!tmpl)
				throw std::runtime_error("template not found in TemplateManager");

			// Construct a new Go using your custom constructor
			Go* go = new Go(*tmpl);  // Uses Go(const TemplateData&, const PlacementData&) constructor
			m_contentdb[templateName] = go;

			std::cout << "Instantiated Go from template: " << templateName << std::endl;
		}
		catch (const std::exception& e)
		{
			logger.WriteF("Failed to instantiate Go from template %s: %s", templateName.c_str(), e.what());
		}
	}*/
}
