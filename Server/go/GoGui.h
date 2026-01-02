#ifndef GOGUI_H_
#define GOGUI_H_

#include "GoComponent.hpp"
#include "../enum/eEquipSlot.hpp"

class GoGui : public GoComponent
{
	public:
		GoGui(Go* go);
		GoGui(Go* go, const TemplateComponent* tmplComp);
		GoGui(Go* go, const std::map<std::string, std::string>& r);

		void Save(MySQL& db);

		void SetEquipRequirements(const string& input);
		eEquipSlot EquipSlot() { return m_equip_slot; }
		map<string, float*> EquipRequirements() { return m_equip_requirements; }
		int InventoryWidth() { return m_inventory_width; }
		int InventoryHeight() { return m_inventory_height; }

		inline void TrimEqRequirements(std::string& s)
		{
			size_t start = s.find_first_not_of(" \t\r\n");
			size_t end = s.find_last_not_of(" \t\r\n");

			if (start == std::string::npos)
			{
				s.clear();
				return;
			}
			s = s.substr(start, end - start + 1);
		}

	private:

		eEquipSlot m_equip_slot;
		int m_inventory_width;
		int m_inventory_height;
		// TODO implement usage
		map<string, float*> m_equip_requirements; // skillname, level
};

#endif