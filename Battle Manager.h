#pragma once
#include <iostream>
#include <windows.h>
#include <vector>
#include <random>
#include <array>
#include <cmath>
#include "Character.h"
#include "ActionQueue.h"
#include "EventBus.h"
#include "ConsoleUtils.h"

#pragma execution_character_set("utf-8")

extern bool Running;

class BattleManager
{
public:
	BattleManager(CharacterTemplate* player, CharacterTemplate* enemy, int player_slots, int enemy_slots)
			: Player(CharacterInstance(player)), Enemy(CharacterInstance(enemy)), player_slots_size(player_slots), enemy_slots_size(enemy_slots) {}
	~BattleManager() = default;

	void EventSub() {
		EventBus::get().subscribe(BattleEvent::BeforeDamage, [this](void* data) {
			// 特色文本Before
			DamageEventData* Data = static_cast<DamageEventData*>(data);
			if (!Data->coin->before.empty())
			{
				setColor(15);
				std::cout << "[台词] " << Data->attacker->Owner->Data->name << "：" << "“" << Data->coin->before << "”\n";
				setColor(8);
			}
			});

		EventBus::get().subscribe(BattleEvent::AfterDamage, [this](void* data) {
			// 特色文本After
			DamageEventData* Data = static_cast<DamageEventData*>(data);
			if (!Data->coin->after.empty())
			{
				setColor(15);
				std::cout << "[台词] " << Data->attacker->Owner->Data->name << "：" << "“" << Data->coin->after << "”\n";
				setColor(8);
			}
			//检测是否越过混乱阈值
			Data->target->Owner->checkConfusion();
			});

		EventBus::get().subscribe(BattleEvent::TurnEnd, [this](void* data) {
			auto dmg = Effect::active::tick(Player.burn);
			if (dmg.has_value()) {
				Player.addHealth(-dmg.value()); 
				setColor(12);
				std::cout << "[烧伤] " << Player.Data->name << " 血量 -" << dmg.value() << "\n";
				setColor(8);
			}
			dmg = Effect::active::tick(Enemy.burn);
			if (dmg.has_value()) {
				Enemy.addHealth(-dmg.value()); 
				setColor(12);
				std::cout << "[烧伤] " << Enemy.Data->name << " 血量 -" << dmg.value() << "\n";
				setColor(8);
			}
			});

		EventBus::get().subscribe(BattleEvent::Damage, [this](void* data) {
			DamageEventData* Data = static_cast<DamageEventData*>(data);
			CharacterInstance* target = nullptr;

			// 处理破裂
			auto dmg = Effect::active::tick(Data->target->Owner->rupture);
			if (dmg.has_value()) {
				Data->target->Owner->addHealth(-dmg.value());
				Data->attacker->selecting.total_damage += dmg.value();
				setColor(10);
				std::cout << "[破裂] " << Data->target->Owner->Data->name << " 血量 -" << dmg.value() << "\n";
				setColor(8);
			}
			// 处理沉沦
			dmg = Effect::active::tick(Data->target->Owner->sink);
			if (dmg.has_value()) {
				Data->target->Owner->addSanity(-dmg.value()); 
				setColor(1);
				std::cout << "[沉沦] " << Data->target->Owner->Data->name << " 理智 -" << dmg.value() << "\n";
				setColor(8);
			}

			// 处理Effect
			if (!Data->coin->effects.empty())
			{
				for (auto& ptr : Data->coin->effects) {

					if (ptr.target == "enemy") { target = Data->target->Owner; }
					else if (ptr.target == "self") { target = Data->attacker->Owner; }
					handleEffect(ptr, target);

					// 重投
					if (ptr.type == "reroll")
					{
						if (Data->coin->reroll_time < ptr.value.y)
						{
							++Data->coin->reroll_time;
							Data->attacker->selecting.coin_list.push_back(*Data->coin);
							std::cout << "[效果] ";
							setColor(15);
							std::cout << "触发重投！\n";
							setColor(8);
						}
					}

					// 震颤引爆
					if (ptr.type == "tremor-explode")
					{
						std::cout << "[效果] ";
						setColor(6);
						std::cout << "震颤引爆！\n";
						setColor(8);
						auto vec = Effect::burst::tremor(target->tremor, target->confusion);
						if (vec.has_value()) {
							target->addHealth(-vec.value().x);
							Data->attacker->selecting.total_damage += vec.value().x;
							target->moveFrontConfusion(vec.value().y);
							setColor(6);
							std::cout << "[震颤] " << Data->target->Owner->Data->name << " 血量 -" << vec.value().x << " 混乱阈值 -" << vec.value().y  << "\n";
							setColor(8);
						}
					}
					// 烧伤爆发
					if (ptr.type == "burn-explode")
					{
						std::cout << "[效果] ";
						setColor(12);
						std::cout << "烧伤爆发！\n";
						setColor(8);
						auto vec = Effect::burst::dot(target->burn);
						if (vec.has_value()) {
							target->addHealth(-vec.value());
							Data->attacker->selecting.total_damage += vec.value();
							setColor(12);
							std::cout << "[烧伤] " << Data->target->Owner->Data->name << " 血量 -" << vec.value() << "\n";
							setColor(8);
						}
					}
					// 沉沦泛滥
					if (ptr.type == "sink-explode")
					{
						std::cout << "[效果] ";
						setColor(1);
						std::cout << "沉沦泛滥！\n";
						setColor(8);
						auto vec = Effect::burst::sink(target->sink, target->sanity, target->Data->sanity);
						if (vec.has_value()) {
							target->addHealth(-vec.value().x);
							Data->attacker->selecting.total_damage += vec.value().x;
							target->addSanity(-vec.value().y);
							setColor(1);
							std::cout << "[沉沦] " << Data->target->Owner->Data->name << " 血量 -" << vec.value().x << " 理智 -" << vec.value().y << "\n";
							setColor(8);
						}
					}
				}
				setColor(8);
			}
			});

		EventBus::get().subscribe(BattleEvent::RollCoin, [this](void* data) {
			CombatEventData* Data = static_cast<CombatEventData*>(data);
			// 处理流血
			auto dmg = Effect::active::tick(Data->a->bleed);
			if (dmg.has_value()) {
				Data->a->addHealth(-dmg.value());
				setColor(4);
				std::cout << "[流血] " << Data->a->Data->name << " 血量 -" << dmg.value() << "\n";
				setColor(8);
			}
			});

		EventBus::get().subscribe(BattleEvent::Critical, [this](void* data) {
			std::cout << "[日志] ";
			setColor(6);
			std::cout << "暴击！\n";
			setColor(8);
			});

		EventBus::get().subscribe(BattleEvent::BeforeCombat, [this](void* data) {
			Action* Data = static_cast<Action*>(data);
			ActionSlot* slots[] = { Data->a, Data->b };
			for (auto* slot : slots) {
				if (slot) {
					// 攻击等级提升
					if (slot->Owner->attack_level_up != Vector2(0, 0))
					{
						auto dmg = Effect::active::tick(slot->Owner->attack_level_up);
						if (dmg.has_value()) {
							slot->selecting.attack_level += dmg.value();
							setColor(4);
							std::cout << "[攻击等级提升] 触发 " << slot->Owner->Data->name << " 本技能攻击等级 +" << dmg.value() << "\n";
							std::cout << "[DEBUG]该技能现攻击等级：" << slot->selecting.attack_level << "\n";
							setColor(8);
						}
					}
					// 攻击等级降低
					if (slot->Owner->attack_level_down != Vector2(0, 0))
					{
						auto dmg = Effect::active::tick(slot->Owner->attack_level_down);
						if (dmg.has_value()) {
							slot->selecting.attack_level -= dmg.value();
							setColor(9);
							std::cout << "[攻击等级降低] 触发 " << slot->Owner->Data->name << " 本技能攻击等级 -" << dmg.value() << "\n";
							if (slot->selecting.attack_level < 1) { slot->selecting.attack_level = 1; }
							std::cout << "[DEBUG]该技能现攻击等级：" << slot->selecting.attack_level << "\n";
							setColor(8);
						}
					}
					// 强壮
					if (slot->Owner->strong != Vector2(0, 0))
					{
						auto dmg = Effect::active::tick(slot->Owner->strong);
						if (dmg.has_value()) {
							slot->selecting.base += dmg.value();
							setColor(4);
							std::cout << "[强壮] 触发 " << slot->Owner->Data->name << " 本技能基础值 +" << dmg.value() << "\n";
							std::cout << "[DEBUG]该技能现基础值：" << slot->selecting.base << "\n";
							setColor(8);
						}
					}
					// 虚弱
					if (slot->Owner->weak != Vector2(0, 0))
					{
						auto dmg = Effect::active::tick(slot->Owner->weak);
						if (dmg.has_value()) {
							slot->selecting.base -= dmg.value();
							setColor(4);
							std::cout << "[虚弱] 触发 " << slot->Owner->Data->name << " 本技能基础值 -" << dmg.value() << "\n";
							if (slot->selecting.base < 0) { slot->selecting.base = 0; }
							std::cout << "[DEBUG]该技能现基础值：" << slot->selecting.base << "\n";
							setColor(8);
						}
					}
				}
			}
			});
	}
	void handleEffect(const CoinEffect& ptr, CharacterInstance* target) {
		// 强壮
		if (ptr.type == "strong")
		{
			Effect::add(target->strong, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(4);
			std::cout << "强壮[" << ptr.value.x << "]\n";
		}
		// 虚弱
		if (ptr.type == "weak")
		{
			Effect::add(target->weak, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(9);
			std::cout << "虚弱[" << ptr.value.x << "]\n";
		}
		// 攻击等级提升
		if (ptr.type == "attack_level_up")
		{
			Effect::add(target->attack_level_up, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(4);
			std::cout << "攻击等级提升[" << ptr.value.x << "]\n";
		}
		// 攻击等级降低
		if (ptr.type == "attack_level_down")
		{
			Effect::add(target->attack_level_down, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(9);
			std::cout << "攻击等级降低[" << ptr.value.x << "]\n";
		}
		if (ptr.type == "burn")
		{
			Effect::add(target->burn, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(12);
			std::cout << "烧伤[" << ptr.value.x << "][" << ptr.value.y << "]\n";
		}
		if (ptr.type == "bleed")
		{
			Effect::add(target->bleed, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(4);
			std::cout << "流血[" << ptr.value.x << "][" << ptr.value.y << "]\n";
		}
		if (ptr.type == "rupture")
		{
			Effect::add(target->rupture, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(10);
			std::cout << "破裂[" << ptr.value.x << "][" << ptr.value.y << "]\n";
		}
		if (ptr.type == "sink")
		{
			Effect::add(target->sink, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(1);
			std::cout << "沉沦[" << ptr.value.x << "][" << ptr.value.y << "]\n";
		}
		if (ptr.type == "tremor")
		{
			Effect::add(target->tremor, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(6);
			std::cout << "震颤[" << ptr.value.x << "][" << ptr.value.y << "]\n";
		}
		if (ptr.type == "breath")
		{
			Effect::add(target->breath, ptr.value);
			std::cout << "[效果] " << target->Data->name << " -> " << "施加 ";
			setColor(15);
			std::cout << "呼吸法[" << ptr.value.x << "][" << ptr.value.y << "]\n";
		}

		if (ptr.type == "sanity")
		{
			target->addSanity(ptr.value.x);
			std::cout << "[效果] 理智加值 " << target->Data->name << " -> " << ptr.value.x << "\n";
		}
		if (ptr.type == "health")
		{
			target->addHealth(ptr.value.x);
			std::cout << "[效果] 血量加值 " << target->Data->name << " -> " << ptr.value.x << "\n";
		}
		setColor(8);
	}

	enum class State
	{
		select,
		combat,
		calc
	};

	void on_enter() {
		for (size_t i = 0; i < player_slots_size; i++)
		{
			Player_Slots.push_back(ActionSlot(&Player));
		}
		for (size_t i = 0; i < enemy_slots_size; i++)
		{
			Enemy_Slots.push_back(ActionSlot(&Enemy));
		}
		EventSub();
		round = 0;
		state = State::select;
	}

	void on_update() {
		switch (state)
		{
		case BattleManager::State::select:
			// 广播回合开始
			EventBus::get().dispatch(BattleEvent::TurnStart, nullptr);
			// 处理混乱，解除混乱
			Player.handleConfusion();
			Enemy.handleConfusion();
			// 敌人默认使用所有行动槽下位的技能
			for (auto& ptr : Enemy_Slots) { ptr.choiceSkill(2); }
			break;
		case BattleManager::State::combat:
			break;
		case BattleManager::State::calc:
			break;
		default:
			break;
		}
	}

	void on_draw() {
		switch (state)
		{
		case BattleManager::State::select:
			// 输出回合数
			std::cout << "回合数： " << round + 1 << "\n\n";
			// 输出双方状态
			showCharacterStatus(Player);
			showEffectStatus(Player);
			showCharacterStatus(Enemy);
			showEffectStatus(Enemy);
			// 若敌人不处于混乱，输出敌人使用的所有技能
			if (!Enemy.isConfused())
			{
				std::cout << Enemy.Data->name << " 正在使用： \n";
				for (auto& ptr : Enemy_Slots)
				{
					showSkillStatus(ptr.selecting);
				}
			}
			// 若敌人不处于混乱，输出所有可用行动槽
			if (!Player.isConfused())
			{
				for (size_t j = 0; j < Player_Slots.size(); j++)
				{
					std::cout << "\n——————————————仪表盘——————————————" << "\n\n";
					for (size_t i = 0; i < Player_Slots[j].Dashboard.size(); i++)
					{
						setColor(15);
						std::cout << i + 1 << ".";
						showSkillStatus(Player_Slots[j].Dashboard[i]);
						std::cout << "\n";
					}
					std::cout << "———————————————————————————————" << "\n";
				}
				std::cout << "请选择技能： ";
				std::cin >> choice;
				for (auto& ptr : Player_Slots)
				{
					ptr.choiceSkill(choice);
				}
			}
			state = State::combat;
			break;
		case BattleManager::State::combat:
			// 重复执行队列最前的action，然后pop掉，直到队列为空
			if (!Player.isConfused() && !Enemy.isConfused())
			{
				for (size_t i = 0; i < Player_Slots.size(); i++)
				{
					action_queue.addAction(ActionType::Combat, Player_Slots[i], Enemy_Slots[i], i);
				}
			}
			else if (Player.isConfused()) // 玩家混乱
			{
				for (size_t i = 0; i < Player_Slots.size(); i++)
				{
					action_queue.addAction(ActionType::Unilateral, Enemy_Slots[i], Player_Slots[i], i);
				}
			}
			else if (Enemy.isConfused())	// 敌人混乱
			{
				for (size_t i = 0; i < Player_Slots.size(); i++)
				{
					action_queue.addAction(ActionType::Unilateral, Player_Slots[i], Enemy_Slots[i], i);
				}
			}
			// 执行
			while (!action_queue.isEmpty())
			{
				action_queue.executeAction();
			}

			//检测死亡，结束游戏
			if (Player.checkDeath() || Enemy.checkDeath())
			{
				setColor(4);
				if (Player.checkDeath()) { std::cout << "[日志]" << Player.Data->name << " 死了！" << "\n"; }
				else { std::cout << "[日志]" << Enemy.Data->name << " 死了！" << "\n"; }
				setColor(8);
				Running = false;
			}
			state = State::calc;
			break;
		case BattleManager::State::calc:
			EventBus::get().dispatch(BattleEvent::TurnEnd, nullptr);
			++round;
			system("pause");
			system("cls");
			state = State::select;
			break;
		default:
			break;
		}
	}

	void on_exit() {}

	void showCharacterStatus(CharacterInstance& ch) {
		setColor(15);
		std::cout << ch.Data->name << " ";
		setColor(8);

		std::cout << "等级：";
		setColor(15);
		std::cout << ch.Data->level << " ";
		setColor(8);

		std::cout << "血量：";
		setColor(15);
		std::cout << ch.health << " / " << ch.Data->health.y << " ";
		setColor(8);

		std::cout << "理智：";
		if (ch.sanity < 0) { setColor(4); }
		else { setColor(9); }
		std::cout << ch.sanity << " ";
		setColor(8);

		std::cout << "当前速度：" << ch.speed << " ";

		std::cout << "最前混乱阈值：";
		if (!ch.confusion.empty()) {
			setColor(6);
			std::cout << (int)(ch.Data->health.y * ch.confusion.front()) << "\n";
			setColor(8);
		}
		else { std::cout << "无\n"; }

		std::cout << "特性关键词：";
		for (size_t i = 0; i < ch.Data->tag_list.size(); i++)
		{
			std::cout << "[" << ch.Data->tag_list[i] << "]";
		}

		if (ch.isConfused())
		{
			setColor(4);
			std::cout << "\n			！陷入混乱！";
			setColor(8);
		}
		std::cout << "\n";
	}

	void showSkillStatus(const Skill& skl) {
		setSinColor(skl.sin_type);
		std::cout << "「" << skl.name << "」";
		for (auto& ptr : skl.coin_list)
		{
			if (ptr.type == "Unbreakable")
			{
				setColor(12);
				std::cout << "■";
			}
			else
			{
				setColor(14);
				std::cout << "●";
			}
			setColor(8);
		}
		setColor(8);
		std::cout << " 攻击等级：";
		setColor(15);
		std::cout << skl.attack_level;
		setColor(8);
		std::cout << " 攻击类型：" << skl.attack_type << " " << "罪孽类型：";
		setSinColor(skl.sin_type);
		std::cout << skl.sin_type << "\n";
		setColor(8);
	}

	void showEffectStatus(CharacterInstance& ch) {
		if (ch.breath != Vector2(0, 0))
		{
			setColor(15);
			std::cout << "呼吸法[" << ch.breath.x << "][" << ch.breath.y << "] ";
		}
		if (ch.burn != Vector2(0, 0))
		{
			setColor(12);
			std::cout << "烧伤[" << ch.burn.x << "][" << ch.burn.y << "] ";
		}
		if (ch.bleed != Vector2(0, 0))
		{
			setColor(4);
			std::cout << "流血[" << ch.bleed.x << "][" << ch.bleed.y << "] ";
		}
		if (ch.rupture != Vector2(0, 0))
		{
			setColor(10);
			std::cout << "破裂[" << ch.rupture.x << "][" << ch.rupture.y << "] ";
		}
		if (ch.sink != Vector2(0, 0))
		{
			setColor(1);
			std::cout << "沉沦[" << ch.sink.x << "][" << ch.sink.y << "] ";
		}
		if (ch.tremor != Vector2(0, 0))
		{
			setColor(14);
			std::cout << "震颤[" << ch.tremor.x << "][" << ch.tremor.y << "]";
		}
		// 状态
		if (ch.attack_level_up != Vector2(0, 0))
		{
			setColor(4);
			std::cout << "攻击等级提升[" << ch.attack_level_up.x << "] ";
		}
		if (ch.attack_level_down != Vector2(0, 0))
		{
			setColor(9);
			std::cout << "攻击等级降低[" << ch.attack_level_down.x << "] ";
		}
		if (ch.strong != Vector2(0, 0))
		{
			setColor(4);
			std::cout << "强壮[" << ch.strong.x << "] ";
		}
		if (ch.weak != Vector2(0, 0))
		{
			setColor(9);
			std::cout << "虚弱[" << ch.weak.x << "] ";
		}
		std::cout << "\n\n";
		setColor(8);
	}

private:
	CharacterInstance Player;
	CharacterInstance Enemy;

	std::vector<ActionSlot> Player_Slots;
	int player_slots_size;
	std::vector<ActionSlot> Enemy_Slots;
	int enemy_slots_size;

	State state;
	int round;
	int choice;
	ActionQueue action_queue;
};