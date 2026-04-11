#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <memory>

#include "Data.h"
#include "Character.h"

class ActionSlot
{
public:
	ActionSlot(CharacterInstance* ch)
		: Owner(ch) {
		fill();
	}
	~ActionSlot() = default;

	CharacterInstance* Owner = nullptr;

	ActionSlot* target = nullptr;

	void setTarget(ActionSlot* slot) { target = slot; }

	void fill() {
		for (size_t i = 0; i < Dashboard.size(); i++)
		{
			Dashboard[i] = Owner->Data->skill_list[RandomManager::get().weight(Owner->Data->skill_weight)];
		}
	}

	void choiceSkill(int num) {
		// 上1，下2
		selecting = Dashboard[num - 1];
		// 如果选择下面的技能，则上面的移下来
		if (num == 2) {
			Dashboard[1] = Dashboard[0];
		}
		Dashboard[0] = Owner->Data->skill_list[RandomManager::get().weight(Owner->Data->skill_weight)];
	}

	Skill selecting;

	std::array<Skill, 2> Dashboard;
};