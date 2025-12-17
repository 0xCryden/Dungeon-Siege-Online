#include "Gas.hpp"
#include "../Common.h"
#include "../helper/Log.h"

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
    unordered_set<string> resolving;

    //size_t totalLoaded = 0;
    function<void(TemplateData&)> resolve = [&](TemplateData& tpl) {
        if (tpl.specializes.empty())
            return;

        if (resolving.count(tpl.name)) {
            cerr << "[ERROR] Circular specialization detected at: " << tpl.name << endl;
            return;
        }

        auto it = templates.find(tpl.specializes);  // <- Use internal templates map
        if (it == templates.end()) {
            cerr << "[ERROR] Missing parent template: " << tpl.specializes << " for " << tpl.name << endl;
            return;
        }

        resolving.insert(tpl.name);
        resolve(it->second);  // Recursively resolve parent first

        for (auto& [compName, parentComp] : it->second.components) {
            //cout << "Merging component: " << compName << " from " << it->first << " into " << tpl.name << endl;
            auto& derivedComp = tpl.components[compName];
            MergeComponent(derivedComp, parentComp);
            //++totalLoaded;
        }

        resolving.erase(tpl.name);
        };

    for (auto& [name, tpl] : templates) {
        resolve(tpl);
    }
    //Log::Write(Log::Level::INFO, "[INFO] Finished resolving inheritances. Total components merged: " + to_string(totalLoaded), true);
}

void TemplateManager::MergeTemplates(TemplateData& child, const string& parent)
{
    MergeTemplates(child, *GetTemplate(parent));
}

// normal template = child, scid template = parent
void TemplateManager::MergeTemplates(TemplateData& child, const TemplateData& parent)
{
    size_t mergedCount = 0;

    for (const auto& [compName, parentComp] : parent.components)
    {
        // If child does not have this component → copy parent component
        if (!child.components.contains(compName))
        {
            child.components[compName] = parentComp;
            ++mergedCount;
            continue;
        }

        // If child *does* have this component → merge, child overrides parent
        auto& childComp = child.components[compName];
        MergeComponent(childComp, parentComp);
        ++mergedCount;
    }

    /*Log::Write(Log::Level::INFO,
        "[INFO] MergeTemplates: Merged " + to_string(mergedCount) +
        " components from parent '" + parent.name +
        "' into child '" + child.scid + "'",
        true);*/
}

Gas::Gas()
{
}

Gas :: ~Gas()
{
}

string Gas::StripLineComment(const string& line) {
    size_t pos = line.find("//");
    return pos != string::npos ? line.substr(0, pos) : line;
}
// --- Trim utility ---
string Gas::Trim(const string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    auto end = s.find_last_not_of(" \t\r\n");
    if (start == string::npos || end == string::npos) return "";
    return s.substr(start, end - start + 1);
}

// --- Extracts component name from "[name]" or "[name*]" ---
string Gas::ParseComponentName(const string& raw)
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

bool Gas::StartsWith(const string& str, const string& prefix) {
    return str.compare(0, prefix.size(), prefix) == 0;
}

void Gas::ParseGasBlock(istream& stream, TemplateComponent& outComp)
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
            if (endPos != string::npos) {
                inBlockComment = false;
                line = line.substr(endPos + 2); // Skip past the block comment end
            }
            else {
                continue; // Still inside block comment — skip line
            }
        }

        // Remove any block comment opening
        size_t blockStart = line.find("/*");
        if (blockStart != string::npos) {
            size_t blockEnd = line.find("*/", blockStart + 2);
            if (blockEnd != string::npos) {
                // Inline block comment: remove it
                line.erase(blockStart, blockEnd - blockStart + 2);
            }
            else {
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
            inserted = move(newComp);
            stack.push_back(&inserted);

            if (braceOpen != string::npos && braceClose != string::npos && braceClose > braceOpen)
            {
                // Extract the content inside { ... }
                string insideBraces = line.substr(braceOpen + 1, braceClose - braceOpen - 1);
                insideBraces = Trim(insideBraces);

                // Now parse fields insideBraces separated by ';'
                size_t start = 0;
                while (start < insideBraces.length())
                {
                    size_t end = insideBraces.find(';', start);
                    string field;
                    if (end == string::npos)
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
                        if (eqPos != string::npos)
                        {
                            string key = Trim(field.substr(0, eqPos));
                            string val = Trim(field.substr(eqPos + 1));
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
        // Block opening after a standalone [name] on previous line
        if (line == "{" && !pendingComponentName.empty())
        {
            TemplateComponent newComp;
            TemplateComponent* parent = stack.back();
            auto& inserted = parent->subcomponents[pendingComponentName];
            inserted = move(newComp);
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

            string typePrefix;
            if (!key.empty() && (key[0] == 'f' || key[0] == 'b' || key[0] == 'i')) // add more prefixes if needed
            {
                if (key.size() > 1 && key[1] == ' ')
                {
                    typePrefix = string(1, key[0]);
                    key = Trim(key.substr(2)); // strip the prefix
                }
            }
            // Handle colon prefix like "common:screen_name"
            size_t colonPos = key.find(':');
            if (colonPos != string::npos)
            {
                string compName = key.substr(0, colonPos);
                string fieldName = key.substr(colonPos + 1);

                TemplateComponent* parent = stack.back();
                auto& comp = parent->subcomponents[compName];
                comp.fields[Trim(fieldName)] = val;
            }
            else
            {
                // Normal field assignment
                stack.back()->fields[key] = val;
            }
            //stack.back()->fields[key] = val;
        }
    }
}

bool Gas::ReadActorPlacements(const string& fullPath,
    vector<PlacementData>& outPlacements)
{
    ifstream file(fullPath);
    if (!file.is_open()) {
        cerr << "[ERROR] Failed to open actor placement file: " << fullPath << endl;
        return false;
    }

    string line;
    while (getline(file, line))
    {
        line = Trim(line);

        // Detect actor header: [t:<name>,n:<instance>]
        if (line.rfind("[t:", 0) == 0 && line.find(",n:") != string::npos)
        {
            PlacementData data;
            data.orientation = { 0,0,0 };
            data.position = SiegePos();

            // ---------------------------------------
            // Parse template name & instance id
            // ---------------------------------------
            size_t tPos = line.find("t:");
            size_t nPos = line.find("n:");
            size_t end = line.find(']');

            data.templateName = Trim(line.substr(tPos + 2, nPos - tPos - 3));
            data.instanceName = Trim(line.substr(nPos + 2, end - nPos - 2));

            // ---------------------------------------
            // Read full actor block into a string
            // ---------------------------------------
            string actorBlock;
            {
                // Seek first '{'
                char ch;
                while (file.get(ch)) {
                    if (ch == '{') {
                        actorBlock += ch;
                        break;
                    }
                }

                // copy until matching brace closes
                int depth = 1;
                while (depth > 0 && file.get(ch)) {
                    actorBlock += ch;
                    if (ch == '{') depth++;
                    else if (ch == '}') depth--;
                }
            }

            // ---------------------------------------
            // Parse using the GAS hierarchy logic
            // ---------------------------------------
            TemplateComponent root;

            try {
                stringstream ss(actorBlock);
                ParseGasBlock(ss, root);
            }
            catch (const exception& e) {
                cerr << "[ERROR] ParseGasBlock failed for actor '"
                    << data.templateName << "': " << e.what() << endl;
                continue;
            }

            // Store full block hierarchy (all subcomponents)
            data.components = root.subcomponents;


            // ---------------------------------------
            // Extract placement fields (if present)
            // ---------------------------------------
            auto itPlace = root.subcomponents.find("placement");
            if (itPlace != root.subcomponents.end())
            {
                auto& pc = itPlace->second;

                if (pc.fields.count("p position"))
                {
                    const string& v = pc.fields.at("p position");
                    float px, py, pz;
                    uint32_t node;

                    if (sscanf_s(v.c_str(), "%f,%f,%f,0x%x", &px, &py, &pz, &node) == 4)
                        data.position = SiegePos(node, px, py, pz);
                }

                if (pc.fields.count("q orientation"))
                {
                    const string& v = pc.fields.at("q orientation");
                    float ox, oy, oz, ow;

                    if (sscanf_s(v.c_str(), "%f,%f,%f,%f", &ox, &oy, &oz, &ow) == 4)
                        data.orientation = { oy, oz, ow };  // your existing behavior
                }
            }


            // ---------------------------------------
            // Extract conversations
            // ---------------------------------------
            auto itConv = root.subcomponents.find("conversation");
            if (itConv != root.subcomponents.end())
            {
                auto itList = itConv->second.subcomponents.find("conversations");
                if (itList != itConv->second.subcomponents.end())
                {
                    // Iterate over all fields inside "conversations"
                    for (const auto& kv : itList->second.fields)
                    {
                        data.conversations.push_back(kv.second);
                    }
                }
            }

            // Push result
            outPlacements.push_back(move(data));
        }
    }

    return true;
}

bool Gas::ReadTemplatesFile(const string& fullPath, unordered_map<string, TemplateData>& outTemplates, const unordered_set<string>& allowedComponents)
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
            transform(tpl.name.begin(), tpl.name.end(), tpl.name.begin(), ::tolower);
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
            }
            catch (const exception& e) {
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

bool Gas::ReadMapTemplatesFile(
    const string& fullPath,
    unordered_map<string, TemplateData>& outTemplates,
    const unordered_set<string>& allowedComponents)
{
    ifstream file(fullPath);
    if (!file.is_open()) {
        cerr << "[ERROR] Failed to open file: " << fullPath << endl;
        return false;
    }

    string line;
    while (getline(file, line))
    {
        line = Trim(line);

        // Accept lines like: [t:<name>,n:<scid>]
        // Reject old-style:  [t:template, ...]
        if (line.rfind("[t:", 0) == 0 && line.rfind("[t:template", 0) != 0)
        {
            TemplateData tpl;

            // Extract "t:<template_name>"
            size_t tPos = line.find("t:");
            if (tPos == string::npos) {
                cerr << "[ERROR] Missing t: field in: " << line << endl;
                continue;
            }
            tPos += 2;

            size_t tEnd = line.find(',', tPos);
            if (tEnd == string::npos) {
                cerr << "[ERROR] Missing ',' after t: in: " << line << endl;
                continue;
            }

            tpl.name = Trim(line.substr(tPos, tEnd - tPos));
            transform(tpl.name.begin(), tpl.name.end(), tpl.name.begin(), ::tolower);

            // Extract "n:<scid>"
            size_t nPos = line.find("n:", tEnd);
            if (nPos == string::npos) {
                cerr << "[ERROR] Missing n: field in: " << line << endl;
                continue;
            }
            nPos += 2;

            size_t nEnd = line.find(']', nPos);
            if (nEnd == string::npos) {
                cerr << "[ERROR] Missing trailing ']' for SCID template in: " << line << endl;
                continue;
            }

            string scid = Trim(line.substr(nPos, nEnd - nPos));
            tpl.scid = scid;   // <-- Add this field to TemplateData if you want to store SCID

            // -------------------------------------
            // Skip to '{'
            // -------------------------------------
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

            // -------------------------------------
            // Parse block
            // -------------------------------------
            TemplateComponent root;
            try {
                ParseGasBlock(file, root);
            }
            catch (const exception& e) {
                cerr << "[ERROR] Exception while parsing block for '" << tpl.name
                    << "': " << e.what() << endl;
                continue;
            }

            tpl.components = move(root.subcomponents);

            // Optional allowlist (commented out in original)
            /*
            if (!allowedComponents.empty()) {
                for (auto it = tpl.components.begin(); it != tpl.components.end(); ) {
                    if (!allowedComponents.contains(it->first)) {
                        it = tpl.components.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
            */

            // Supports inheritance and doc fields if provided
            if (root.fields.count("specializes"))
                tpl.specializes = root.fields["specializes"];

            if (root.fields.count("doc"))
                tpl.doc = root.fields["doc"];

            // -------------------------------------
            // Store new template
            // -------------------------------------
            outTemplates[tpl.scid] = move(tpl);
        }
    }

    if (outTemplates.empty()) {
        //cerr << "[WARN] No SCID templates found in file: " << fullPath << endl;
        //return false;
        return true;
    }

    return true;
}


void Gas::LogComponent(const string& name, const TemplateComponent& comp, const string& indent)
{
    cout << indent << "- [" << name << "] (fields: " << comp.fields.size()
        << ", subcomponents: " << comp.subcomponents.size() << ")" << endl;

    for (const auto& [key, value] : comp.fields)
    {
        cout << indent << "    " << key << " = " << value << endl;
    }

    for (const auto& [subName, subComp] : comp.subcomponents)
    {
        LogComponent(subName, subComp, indent + "  ");
    }
}

void Gas::LoadTemplates()
{
    unordered_set<string> allowed = { "actor", "aspect", "mind" };
    const fs::path tanksRoot = "resources";

    size_t totalLoaded = 0;

    // Iterate all tank folders
    for (const auto& tankEntry : fs::directory_iterator(tanksRoot))
    {
        if (!tankEntry.is_directory())
            continue;

        const fs::path tankRoot = tankEntry.path();

        vector<fs::path> orderedDirs;
        fs::path coreDir;
        fs::path interactiveDir;

        // Collect first-level folders inside the tank
        for (const auto& entry : fs::directory_iterator(tankRoot))
        {
            if (!entry.is_directory())
                continue;

            const string name = entry.path().filename().string();

            if (name == "_core")
                coreDir = entry.path();
            else if (name == "interactive")
                interactiveDir = entry.path();
            else
                orderedDirs.push_back(entry.path());
        }

        // Deterministic order for remaining folders
        sort(orderedDirs.begin(), orderedDirs.end());

        vector<fs::path> loadOrder;

        if (!coreDir.empty())
            loadOrder.push_back(coreDir);

        if (!interactiveDir.empty())
            loadOrder.push_back(interactiveDir);

        loadOrder.insert(loadOrder.end(), orderedDirs.begin(), orderedDirs.end());

        // Load templates in enforced order for this tank
        for (const auto& dir : loadOrder)
        {
            for (const auto& entry : fs::recursive_directory_iterator(dir))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".gas")
                {
                    unordered_map<string, TemplateData> fileTemplates;

                    if (ReadTemplatesFile(entry.path().string(), fileTemplates, allowed))
                    {
                        for (auto& [name, tpl] : fileTemplates)
                        {
                            manager.AddTemplate(std::move(tpl));
                            ++totalLoaded;
                        }
                    }
                }
            }
        }
    }

    manager.ResolveTemplateInheritance();

    Log::Write(
        Log::Level::INFO,
        to_string(totalLoaded) + " templates loaded",
        true
    );
}

void Gas::LoadMapTemplates()
{
    const fs::path mapsRoot = "maps";
    size_t totalLoaded = 0;

    if (!fs::exists(mapsRoot) || !fs::is_directory(mapsRoot))
    {
        Log::Write(Log::Level::ERR, "Maps directory not found: maps/", true);
        return;
    }

    // Iterate over maps/<mapName>/
    for (const auto& mapEntry : fs::directory_iterator(mapsRoot))
    {
        if (!mapEntry.is_directory())
            continue;

        fs::path regionsPath = mapEntry.path() / "regions";
        if (!fs::exists(regionsPath) || !fs::is_directory(regionsPath))
            continue;

        // Scan maps/<mapName>/regions/**/*
        for (const auto& entry : fs::recursive_directory_iterator(regionsPath))
        {
            if (!entry.is_regular_file())
                continue;

            const fs::path& path = entry.path();

            if (path.extension() != ".gas" || path.filename() != "actor.gas")
                continue;

            auto parent = path.parent_path();        // regular
            auto grandparent = parent.parent_path();      // objects
            auto regionFolder = grandparent.parent_path();// <REGION>

            // Enforce structure: <REGION>/objects/regular/actor.gas
            if (parent.filename() != "regular" ||
                grandparent.filename() != "objects")
                continue;

            string regionName = regionFolder.filename().string();
            string fullPath = path.string();

            unordered_map<string, TemplateData> fileTemplates;
            unordered_set<string> allowedComponents;

            if (!ReadMapTemplatesFile(fullPath, fileTemplates, allowedComponents))
            {
                Log::Write(Log::Level::ERR,
                    "Failed to load SCID templates from " + fullPath, true);
                continue;
            }

            for (auto& [scid, tpl] : fileTemplates)
            {
                tpl.region = regionName;
                manager.mapTemplates[scid] = tpl;
                ++totalLoaded;
            }
        }
    }

    Log::Write(Log::Level::INFO,
        to_string(totalLoaded) + " SCID templates loaded", true);

    // -----------------------------------------
    // Merge SCID templates → normal templates
    // -----------------------------------------
    size_t mergeCount = 0;

    for (auto& [scid, scidTpl] : manager.mapTemplates)
    {
        TemplateData* normalTpl = manager.GetTemplate(scidTpl.name);
        if (!normalTpl)
        {
            cerr << "[WARN] SCID template '" << scid
                << "' (region: " << scidTpl.region
                << ") has no matching base template." << endl;
            continue;
        }

        manager.MergeTemplates(scidTpl, *normalTpl);
        ++mergeCount;
    }

    Log::Write(Log::Level::INFO,
        to_string(mergeCount) + " SCID templates updated", true);
}
