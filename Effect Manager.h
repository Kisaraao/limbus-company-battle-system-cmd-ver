#pragma once
#include <iostream>

#include "ConsoleUtils.h"
#include "ActionSlot.h"
#include "Effect.h"

class EffectManager
{
public:

	static EffectManager& Get() {
		static EffectManager instance;
		return instance;
	}

	void addEffect(Vector2& vec, CoinEffect& ptr, CharacterInstance& target, const char* name, int color) {
		Effect::add(vec, ptr.value);
		std::cout << "[效果] " << target.Data->name << " <- " << "施加 ";
		setColor(color);
		std::cout << name << "[" << ptr.value.x << "]" << "[" << ptr.value.y << "]\n";
		setColor(8);
	}

	void handleStatus(ActionSlot& slot) {
		// 攻击等级提升
		if (slot.Owner->attack_level_up != Vector2(0, 0))
		{
			auto dmg = Effect::active::tick(slot.Owner->attack_level_up);
			if (dmg.has_value()) {
				slot.selecting.attack_level += dmg.value();
				setColor(4);
				std::cout << "[攻击等级提升] 触发 " << slot.Owner->Data->name << " 本技能攻击等级 +" << dmg.value() << "\n";
				setColor(8);
			}
		}
		// 攻击等级降低
		if (slot.Owner->attack_level_down != Vector2(0, 0))
		{
			auto dmg = Effect::active::tick(slot.Owner->attack_level_down);
			if (dmg.has_value()) {
				slot.selecting.attack_level -= dmg.value();
				setColor(9);
				std::cout << "[攻击等级降低] 触发 " << slot.Owner->Data->name << " 本技能攻击等级 -" << dmg.value() << "\n";
				if (slot.selecting.attack_level < 1) { slot.selecting.attack_level = 1; }
				setColor(8);
			}
		}
		// 强壮
		if (slot.Owner->strong != Vector2(0, 0))
		{
			auto dmg = Effect::active::tick(slot.Owner->strong);
			if (dmg.has_value()) {
				slot.selecting.base += dmg.value();
				setColor(4);
				std::cout << "[强壮] 触发 " << slot.Owner->Data->name << " 本技能基础值 +" << dmg.value() << "\n";
				setColor(8);
			}
		}
		// 虚弱
		if (slot.Owner->weak != Vector2(0, 0))
		{
			auto dmg = Effect::active::tick(slot.Owner->weak);
			if (dmg.has_value()) {
				slot.selecting.base -= dmg.value();
				setColor(4);
				std::cout << "[虚弱] 触发 " << slot.Owner->Data->name << " 本技能基础值 -" << dmg.value() << "\n";
				if (slot.selecting.base < 0) { slot.selecting.base = 0; }
				setColor(8);
			}
		}
	}

private:

};