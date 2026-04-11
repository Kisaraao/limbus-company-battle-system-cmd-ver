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
#include "Effect Manager.h"

#pragma execution_character_set("utf-8")

extern bool Running;

void addEffect(CoinEffect& ptr, CharacterInstance& target) {
	if (ptr.type == "fragile") { EffectManager::Get().addEffect(target.fragile, ptr, target, "易损", 5); target.rounder_fragile = 0; }
	if (ptr.type == "protect") { EffectManager::Get().addEffect(target.protect, ptr, target, "守护", 9); target.rounder_protect = 0; }
	if (ptr.type == "damage_enhance") { EffectManager::Get().addEffect(target.damage_enhance, ptr, target, "伤害强化", 4); target.rounder_enhance = 0; }
	if (ptr.type == "damage_weak") { EffectManager::Get().addEffect(target.damage_weak, ptr, target, "伤害弱化", 9); target.rounder_weak = 0; }
	if (ptr.type == "strong") { EffectManager::Get().addEffect(target.strong, ptr, target, "强壮", 4); }
	if (ptr.type == "weak") { EffectManager::Get().addEffect(target.weak, ptr, target, "虚弱", 9); }
	if (ptr.type == "attack_level_up") { EffectManager::Get().addEffect(target.attack_level_up, ptr, target, "攻击等级提升", 4); }
	if (ptr.type == "attack_level_down") { EffectManager::Get().addEffect(target.attack_level_down, ptr, target, "攻击等级降低", 9); }

	if (ptr.type == "burn") { EffectManager::Get().addEffect(target.burn, ptr, target, "烧伤", 12); }
	if (ptr.type == "bleed") { EffectManager::Get().addEffect(target.bleed, ptr, target, "流血", 4); }
	if (ptr.type == "rupture") { EffectManager::Get().addEffect(target.rupture, ptr, target, "破裂", 10); }
	if (ptr.type == "sink") { EffectManager::Get().addEffect(target.sink, ptr, target, "沉沦", 1); }
	if (ptr.type == "tremor") { EffectManager::Get().addEffect(target.tremor, ptr, target, "震颤", 6); }
	if (ptr.type == "breath") { EffectManager::Get().addEffect(target.breath, ptr, target, "呼吸法", 15); }
	if (ptr.type == "charge") { EffectManager::Get().addEffect(target.charge, ptr, target, "充能", 3); }

	if (ptr.type == "sanity")
	{
		target.addSanity(ptr.value.x);
		std::cout << "[效果] 理智加值 " << target.Data->name << " << " << ptr.value.x << "\n";
	}
	if (ptr.type == "health")
	{
		target.addHealth(ptr.value.x);
		std::cout << "[效果] 血量加值 " << target.Data->name << " << " << ptr.value.x << "\n";
	}
}

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
			Player.ClearRounders();
			Enemy.ClearRounders();

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
			// 处理流血
			dmg = Effect::active::tick(Data->attacker->Owner->bleed);
			if (dmg.has_value()) {
				Data->attacker->Owner->addHealth(-dmg.value());
				setColor(4);
				std::cout << "[流血] " << Data->attacker->Owner->Data->name << " 血量 -" << dmg.value() << "\n";
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

					if (ptr.target == "self") {  target = Data->attacker->Owner;}
					else  { target = Data->target->Owner; }

					addEffect(ptr, *target);

					// 重投
					if (ptr.type == "reroll")
					{
						if (Data->coin->reroll_time < ptr.value.y)
						{
							++Data->coin->reroll_time;
							Coin temp = *Data->coin;
							temp.before.clear();
							temp.after.clear();
							Data->attacker->selecting.coin_list.insert(Data->attacker->selecting.coin_list.begin() + Data->coin_index + 1, temp);
							std::cout << "[效果] ";
							setColor(15);
							std::cout << "硬币重投！\n";
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
							if (vec.value().y != 0){ target->moveFrontConfusion(vec.value().y); }
							setColor(6);
							std::cout << "[震颤引爆] " << target->Data->name << " 血量 -" << vec.value().x << " 混乱阈值 -" << vec.value().y  << "\n";
							setColor(8);
						}
					}
					// 裂痕崩坏
					if (ptr.type == "rupture-explode")
					{
						std::cout << "[效果] ";
						setColor(10);
						std::cout << "裂痕崩坏！\n";
						setColor(8);
						auto vec = Effect::burst::dot(target->rupture);
						if (vec.has_value()) {
							target->addHealth(-vec.value());
							Data->attacker->selecting.total_damage += vec.value();
							setColor(10);
							std::cout << "[裂痕崩坏] " << target->Data->name << " 血量 -" << vec.value() << "\n";
							setColor(8);
						}
					}
					// 大出血
					if (ptr.type == "bleed-explode")
					{
						std::cout << "[效果] ";
						setColor(4);
						std::cout << "大出血！\n";
						setColor(8);
						auto vec = Effect::burst::dot(target->bleed);
						if (vec.has_value()) {
							target->addHealth(-vec.value());
							Data->attacker->selecting.total_damage += vec.value();
							setColor(4);
							std::cout << "[大出血] " << target->Data->name << " 血量 -" << vec.value() << "\n";
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
							std::cout << "[烧伤爆发] " << target->Data->name << " 血量 -" << vec.value() << "\n";
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
							std::cout << "[沉沦泛滥] " << target->Data->name << " 血量 -" << vec.value().x << " 理智 -" << vec.value().y << "\n";
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

			dmg = Effect::active::tick(Data->b->bleed);
			if (dmg.has_value()) {
				Data->b->addHealth(-dmg.value());
				setColor(4);
				std::cout << "[流血] " << Data->b->Data->name << " 血量 -" << dmg.value() << "\n";
				setColor(8);
			}
			});

		EventBus::get().subscribe(BattleEvent::Critical, [this](void* data) {
			std::cout << "[日志] ";
			setColor(6);
			std::cout << "暴击！\n";
			setColor(8);
			});

		EventBus::get().subscribe(BattleEvent::UsingSkill, [this](void* data) {
			UsingSkillEventData* Data = static_cast<UsingSkillEventData*>(data);
			
			std::unordered_map<std::string, Vector2> target_effect_map = {
				{ "bleed", Data->target->Owner->bleed },
				{ "burn", Data->target->Owner->burn },
				{ "rupture", Data->target->Owner->rupture },
				{ "sink", Data->target->Owner->sink },
				{ "tremor", Data->target->Owner->tremor },
				{ "breath", Data->target->Owner->breath },
				{ "charge", Data->target->Owner->charge },
				{ "health", Vector2(Data->target->Owner->health, 0) },
				{ "sanity", Vector2(Data->target->Owner->sanity, 0) },
				{ "fragile", Data->target->Owner->fragile },
				{ "protect", Data->target->Owner->protect },
				{ "damage_enhance", Data->target->Owner->damage_enhance },
				{ "damage_weak", Data->target->Owner->damage_weak },
				{ "strong", Data->target->Owner->strong },
				{ "weak", Data->target->Owner->weak },
				{ "attack_level_up", Data->target->Owner->attack_level_up },
				{ "attack_level_down", Data->target->Owner->attack_level_down }
			};

			std::unordered_map<std::string, Vector2> self_effect_map = {
				{ "bleed", Data->self->Owner->bleed },
				{ "burn", Data->self->Owner->burn },
				{ "rupture", Data->self->Owner->rupture },
				{ "sink", Data->self->Owner->sink },
				{ "tremor", Data->self->Owner->tremor },
				{ "breath", Data->self->Owner->breath },
				{ "charge", Data->self->Owner->charge },
				{ "health", Vector2(Data->self->Owner->health, 0) },
				{ "sanity", Vector2(Data->self->Owner->sanity, 0) },
				{ "fragile", Data->self->Owner->fragile },
				{ "protect", Data->self->Owner->protect },
				{ "damage_enhance", Data->self->Owner->damage_enhance },
				{ "damage_weak", Data->self->Owner->damage_weak },
				{ "strong", Data->self->Owner->strong },
				{ "weak", Data->self->Owner->weak },
				{ "attack_level_up", Data->self->Owner->attack_level_up },
				{ "attack_level_down", Data->self->Owner->attack_level_down }
			};

			// 技能进化
			if (!Data->self->selecting.change_list.empty())
			{
				for (auto& change : Data->self->selecting.change_list) {
					bool can_develop = false;
					Vector2& effect = change.target == "enemy" ? target_effect_map[change.effect] : self_effect_map[change.effect];

					if (change.vec == "x" && cmpMap[change.mode](effect.x, change.value)) { can_develop = true; }
					if (change.vec == "y" && cmpMap[change.mode](effect.y, change.value)) { can_develop = true; }

					if (can_develop) {
						if (change.notice) {
							setColor(14);
							std::cout << "[技能进化] 满足条件，本技能进化。" << "\n";
							setColor(8);
						}
						Data->self->selecting = Data->self->Owner->Data->skill_list[change.skill];
						break;
					}
				}
			}
			
			// 特效
			for (auto& ptr : Data->self->selecting.using_list)
			{
				if (ptr.target == "self") { addEffect(ptr, *Data->self->Owner); }
				else { addEffect(ptr, *Data->target->Owner); }
			}

			// 增益
			if (!Data->self->selecting.enhance_list.empty()) {
				for (auto& enhance : Data->self->selecting.enhance_list)
				{
					bool can_enhance = false;
					Vector2& compare_effect = enhance.compare_target == "enemy" ? target_effect_map[enhance.compare_effect] : self_effect_map[enhance.compare_effect];
				
					if (enhance.compare_vec == "x" && cmpMap[enhance.mode](compare_effect.x, enhance.compare_value)) { can_enhance = true; }
					if (enhance.compare_vec == "y" && cmpMap[enhance.mode](compare_effect.y, enhance.compare_value)) { can_enhance = true; }
				
					if (can_enhance) {
						if (enhance.notice) {
							setColor(12);
							std::cout << "[技能强化] 满足条件，本技能强化。" << "\n";
							setColor(8);
						}
						
						CoinEffect enhance_effect;
						if (enhance.enhance_vec == "x") 
						{
							enhance_effect.type = enhance.enhance_effect;
							enhance_effect.value = Vector2(enhance.enhance_value, 0);
							addEffect(enhance_effect, *Data->self->Owner);
						}
						if (enhance.enhance_vec == "y") 
						{
							enhance_effect.type = enhance.enhance_effect;
							enhance_effect.value = Vector2(0, enhance.enhance_value);
							addEffect(enhance_effect, *Data->self->Owner);
						}
					
						if (enhance.has_cost) {
							CoinEffect cost_effect;
							if (enhance.cost_vec == "x")
							{
								cost_effect.type = enhance.cost_effect;
								cost_effect.value = Vector2(enhance.cost_value, 0);
								addEffect(cost_effect, *Data->self->Owner);
							}
							if (enhance.cost_vec == "y")
							{
								cost_effect.type = enhance.cost_effect;
								cost_effect.value = Vector2(0, enhance.cost_value);
								addEffect(cost_effect, *Data->self->Owner);
							}
						}
					}
				}
			}

			// using 特色文本
			if (!Data->self->selecting.using_speak.empty())
			{
				setColor(15);
				std::cout << "[台词] " << Data->self->Owner->Data->name << "：" << "“" << Data->self->selecting.using_speak << "”\n";
				setColor(8);
			}
			});

		EventBus::get().subscribe(BattleEvent::BeforeCombat, [this](void* data) {
			Action* Data = static_cast<Action*>(data);
			EffectManager::Get().handleStatus(*Data->a);
			EffectManager::Get().handleStatus(*Data->b);
			});

		EventBus::get().subscribe(BattleEvent::AfterCombat, [this](void* data) {
			AfterCombatEventData* Data = static_cast<AfterCombatEventData*>(data);
			// winner 特色文本
			if (!Data->winner->selecting.combat_win_speak.empty())
			{
				setColor(15);
				std::cout << "[台词] " << Data->winner->Owner->Data->name << "：" << "“" << Data->winner->selecting.combat_win_speak << "”\n";
				setColor(8);
			}
			// loser 特色文本
			if (!Data->loser->selecting.combat_lose_speak.empty())
			{
				setColor(15);
				std::cout << "[台词] " << Data->loser->Owner->Data->name << "：" << "“" << Data->loser->selecting.combat_lose_speak << "”\n";
				setColor(8);
			}

			for (auto& ptr : Data->winner->selecting.combat_win)
			{
				if (ptr.target == "self") {
					addEffect(ptr, *Data->winner->Owner);
				}
				else {
					addEffect(ptr, *Data->loser->Owner);
				}
			}
			for (auto& ptr : Data->loser->selecting.combat_lose)
			{
				if (ptr.target == "self") {
					addEffect(ptr, *Data->loser->Owner);
				}
				else {
					addEffect(ptr, *Data->winner->Owner);
				}
			}
			std::cout << "\n";
			});
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

		// 特效
		for (auto ptr : Player.Data->on_game_start_effect_list) { addEffect(ptr, Player); }
		for (auto ptr : Enemy.Data->on_game_start_effect_list) { addEffect(ptr, Enemy); }

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
					setColor(15);
					std::cout << "\n+--------------------------------DashBoard---------------------------------+" << "\n";
					setColor(8);
					for (size_t i = 0; i < Player_Slots[j].Dashboard.size(); i++)
					{
						setColor(15);
						showSkillStatus(Player_Slots[j].Dashboard[i]);
						std::cout << "";
					}
					setColor(15);
					std::cout << "+--------------------------------------------------------------------------+" << "\n";
					setColor(8);
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

		std::cout << "等级:";
		setColor(15);
		std::cout << ch.Data->level << " ";
		setColor(8);

		std::cout << "血量:";
		setColor(15);
		std::cout << "[" << ch.health << "/" << ch.Data->health.y << "] ";
		setColor(8);

		std::cout << "理智:";
		if (ch.sanity < 0) { setColor(4); }
		else { setColor(9); }
		std::cout << ch.sanity << " ";
		setColor(8);

		//std::cout << "当前速度：" << ch.speed << " ";

		std::cout << "最前混乱阈值:";
		if (!ch.confusion.empty()) {
			setColor(6);
			std::cout << (int)(ch.Data->health.y * ch.confusion.front()) << "\n";
			setColor(8);
		}
		else { std::cout << "无\n"; }

		std::cout << "特性关键词:";
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
		std::cout << " <" << skl.name << "> ";
		for (auto& ptr : skl.coin_list)
		{
			if (ptr.type == "Unbreakable")
			{
				setColor(4);
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
		std::cout << " 攻击等级:";
		setColor(15);
		std::cout << skl.attack_level;
		setColor(8);
		std::cout << " 基础值:";
		setColor(15);
		std::cout << skl.base;
		setColor(8);
		std::cout << " 变动值:";
		setColor(15);
		std::cout << skl.change;
		setColor(8);
		std::cout << " 攻击类型:";
		setColor(15);
		std::cout << skl.attack_type;
		setColor(8);
		std::cout << " 罪孽类型:";
		setSinColor(skl.sin_type);
		std::cout << skl.sin_type;
		setColor(8);
		std::cout << "\n";
	}

	void showEffectStatus(CharacterInstance& ch) {
		if (ch.charge != Vector2(0, 0))
		{
			setColor(ch.Data->custom_charge.enable ? ch.Data->custom_charge.color : 3);
			std::cout << ch.Data->custom_charge.name;
			std::cout << "[" << ch.charge.x << "]";
			if ((ch.Data->custom_charge.enable && ch.Data->custom_charge.double_count) || !ch.Data->custom_charge.enable) {
				std::cout << "[" << ch.charge.y << "]";
			}
			std::cout << " ";
		}
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
			std::cout << "震颤[" << ch.tremor.x << "][" << ch.tremor.y << "] ";
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
		if (ch.damage_enhance != Vector2(0, 0))
		{
			setColor(4);
			std::cout << "伤害强化[" << ch.damage_enhance.x << "] ";
		}
		if (ch.damage_weak != Vector2(0, 0))
		{
			setColor(9);
			std::cout << "伤害弱化[" << ch.damage_weak.x << "] ";
		}
		if (ch.protect != Vector2(0, 0))
		{
			setColor(9);
			std::cout << "守护[" << ch.protect.x << "] ";
		}
		if (ch.fragile != Vector2(0, 0))
		{
			setColor(5);
			std::cout << "易损[" << ch.fragile.x << "] ";
		}
		std::cout << "\n\n";
		setColor(8);
	}

private:
	CharacterInstance Player;
	CharacterInstance Enemy;

	int player_slots_size;
	int enemy_slots_size;

	ActionQueue action_queue;

	std::vector<ActionSlot> Player_Slots;
	std::vector<ActionSlot> Enemy_Slots;

	State state;
	int round;
	int choice;
};