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
	BattleManager(CharacterData* player, CharacterData* enemy) {
		Player.loadCharacterData(player);
		Enemy.loadCharacterData(enemy);

		round = 0;
		for (size_t i = 0; i < dashboard.size(); i++)
		{
			Player.rollWeight();
			dashboard[i] = Player.weight;
		}

		EventBus::get().subscribe(BattleEvent::BeforeDamage, [this](void* data) {
			// 特色文本Before
			DamageEventData* Data = static_cast<DamageEventData*>(data);
			if (!Data->coin->before.empty())
			{
				setColor(15);
				std::cout << "[台词] " << Data->attacker->Data->Name << "：" << "“" << Data->coin->before << "”\n";
				setColor(8);
			}
			});

		EventBus::get().subscribe(BattleEvent::AfterDamage, [this](void* data) {
			// 特色文本After
			DamageEventData* Data = static_cast<DamageEventData*>(data);
			if (!Data->coin->after.empty())
			{
				setColor(15);
				std::cout << "[台词] " << Data->attacker->Data->Name << "：" << "“" << Data->coin->after << "”\n";
				setColor(8);
			}
			});

		EventBus::get().subscribe(BattleEvent::TurnEnd, [this](void* data) {
			Player.handleBurn();
			Enemy.handleBurn();
			});

		EventBus::get().subscribe(BattleEvent::Damage, [this](void* data) {
			DamageEventData* Data = static_cast<DamageEventData*>(data);
			BattleCharacter* target = nullptr;

			*Data->total_damage += Data->target->handleRupture();
			Data->target->handleSink();

			// 处理Effect
			if (!Data->coin->effects.empty())
			{
				for (auto& ptr : Data->coin->effects) {

					if (ptr.target == "enemy") { target = Data->target; } else if (ptr.target == "self") { target = Data->attacker; }

					if (ptr.type == "burn")
					{
						target->addBurn(ptr.value);
						std::cout << "[效果] 施加烧伤：" << target->Data->Name << " -> " << " 强度：" << ptr.value.x << " 层数：" << ptr.value.y << "\n";
					}
					if (ptr.type == "bleed")
					{
						target->addBleed(ptr.value);
						std::cout << "[效果] 施加流血：" << target->Data->Name << " -> " << " 强度：" << ptr.value.x << " 层数：" << ptr.value.y << "\n";
					}
					if (ptr.type == "rupture")
					{
						target->addRupture(ptr.value);
						std::cout << "[效果] 施加破裂：" << target->Data->Name << " -> " << " 强度：" << ptr.value.x << " 层数：" << ptr.value.y << "\n";
					}
					if (ptr.type == "sink")
					{
						target->addSink(ptr.value);
						std::cout << "[效果] 施加沉沦：" << target->Data->Name << " -> " << " 强度：" << ptr.value.x << " 层数：" << ptr.value.y << "\n";
					}
					if (ptr.type == "tremor")
					{
						target->addTremor(ptr.value);
						std::cout << "[效果] 施加震颤：" << target->Data->Name << " -> " << " 强度：" << ptr.value.x << " 层数：" << ptr.value.y << "\n";
					}
					if (ptr.type == "breath")
					{
						target->addBreath(ptr.value);
						std::cout << "[效果] 施加呼吸法：" << target->Data->Name << " -> " << " 强度：" << ptr.value.x << " 层数：" << ptr.value.y << "\n";
					}

					if (ptr.type == "sanity")
					{
						target->addSanity(ptr.value.x);
						std::cout << "[效果] 理智加值：" << target->Data->Name << " -> " << ptr.value.x << "\n";
					}
					if (ptr.type == "health")
					{
						target->addHealth(ptr.value.x);
						std::cout << "[效果] 血量加值：" << target->Data->Name << " -> " << ptr.value.x << "\n";
					}

					if (ptr.type == "tremor-explode")
					{
						std::cout << "[效果] 震颤引爆：" << target->Data->Name << "\n";
						*Data->total_damage += target->handleTremor();
					}
					if (ptr.type == "burn-explode")
					{
						std::cout << "[效果] 烧伤引爆：" << target->Data->Name << "\n";
						*Data->total_damage += target->burnExplode();
					}
				}
			}
			});

		EventBus::get().subscribe(BattleEvent::rollCoin, [this](void* data) {
			DamageEventData* Data = static_cast<DamageEventData*>(data);
			Data->attacker->handleBleed();
			});

		EventBus::get().subscribe(BattleEvent::Critical, [this](void* data) {
			std::cout << "[日志] " << "暴击！\n";
			});
	}
	~BattleManager() = default;

	enum class State
	{
		input,
		combat,
		calc
	};

	void on_enter() {
		setColor(8);
		switch (state)
		{
		case BattleManager::State::input:
			EventBus::get().dispatch(BattleEvent::TurnStart, nullptr);

			//如果处于混乱，解除
			if (Player.status == Status::confusion)
			{
				Player.status = Status::normal;
			}
			if (Enemy.status == Status::confusion)
			{
				Enemy.status = Status::normal;
			}
			//检测是否越过混乱阈值
			Player.checkConfusion();
			Enemy.checkConfusion();

			std::cout << "回合数： " << round + 1 << "\n\n";
			break;
		case BattleManager::State::combat:
			break;
		case BattleManager::State::calc:
			break;
		default:
			break;
		}
	}

	void on_update() {
		switch (state)
		{
		case BattleManager::State::input:
			Enemy.rollWeight();
			Enemy.choice = Enemy.weight;
			Enemy.setCurrentSkill(Enemy.choice);
			if (round == 0)
			{
				for (size_t i = 0; i < dashboard.size(); i++)
				{
					Player.rollWeight();
					dashboard[i] = Player.weight;
				}
			}
			else 
			{
				if (choice == 1) { dashboard[1] = dashboard[0]; }
				Player.rollWeight();
				dashboard[0] = Player.weight;
			}
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
		case BattleManager::State::input:
			showCharacterStatus(Player);
			showEffectStatus(Player);
			showCharacterStatus(Enemy);
			showEffectStatus(Enemy);

			if (Enemy.status != Status::confusion)
			{
				std::cout << Enemy.Data->Name << " 正在使用： \n";
				showSkillStatus(Enemy.skill);
			}

			if (Player.status != Status::confusion)
			{
				std::cout << "\n——————————————仪表盘——————————————" << "\n\n";
				for (size_t i = 0; i < dashboard.size(); i++)
				{
					setColor(15);
					std::cout << i + 1 << ". ";
					showSkillStatus(Player.Data->Skill_List[dashboard[i]]);
					std::cout << "\n";
				}
				std::cout << "———————————————————————————————" << "\n";
				std::cout << "请选择技能： ";
				std::cin >> choice;
				choice--;
				Player.choice = dashboard[choice];
				Player.setCurrentSkill(Player.choice);
			}
			break;
		case BattleManager::State::combat:
			//重复执行队列最前的action，然后pop掉，直到队列为空

			if (Player.status != Status::confusion && Enemy.status != Status::confusion)
			{
				action_queue.addAction(ActionType::Combat, Player, Enemy, 1);
			}
			else if (Player.status == Status::confusion)		//玩家混乱
			{
				action_queue.addAction(ActionType::Unilateral, Enemy, Player, 1);
			}
			else if (Enemy.status == Status::confusion)	//敌人混乱
			{
				action_queue.addAction(ActionType::Unilateral, Player, Enemy, 1);
			}
			while (!action_queue.isEmpty())
			{
				action_queue.executeAction();
			}
			break;
		case BattleManager::State::calc:
			break;
		default:
			break;
		}
	}

	void on_exit() {
		switch (state)
		{
		case BattleManager::State::input:
			state = State::combat;
			break;
		case BattleManager::State::combat:
			//检测死亡，结束游戏
			if (Player.checkDeath() || Enemy.checkDeath())
			{
				setColor(4);
				if (Player.checkDeath())
				{
					std::cout << "[日志]" << Player.Data->Name << " 死了！" << "\n";
				}
				else
				{
					std::cout << "[日志]" << Enemy.Data->Name << " 死了！" << "\n";
				}
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
			state = State::input;
			break;
		default:
			break;
		}
	}

	void showCharacterStatus(const BattleCharacter& ch) {
		setColor(15);
		std::cout << ch.Data->Name << " ";
		setColor(8);
		std::cout << "等级： ";
		setColor(15);
		std::cout << ch.Data->Level << " ";
		setColor(8);
		std::cout << "当前血量： ";
		setColor(15);
		std::cout << ch.health << " / " << ch.Data->Health.y << " ";
		setColor(8);
		std::cout << "当前理智： ";
		if (ch.sanity < 0)
		{
			setColor(4);
		}else
		{
			setColor(9);
		}
		std::cout << ch.sanity << " ";
		setColor(8);
		std::cout << "当前速度： " << ch.speed << "\n";
		std::cout << "特性关键词： ";
		for (size_t i = 0; i < ch.Data->Tag_List.size(); i++)
		{
			std::cout << "[" << ch.Data->Tag_List[i] << "] ";
		}
		std::cout << "下一次混乱： ";
		if (!ch.confusion.empty()) {
			std::cout << ch.Data->Health.y * ch.confusion.front();
		}else {
			std::cout << "无";
		}
		if (ch.status == Status::confusion)
		{
			setColor(4);
			std::cout << "\n！陷入混乱！";
			setColor(8);
		}
		std::cout << "\n";
	}

	void showSkillStatus(const Skill& skl) {
		setSinColor(skl.Sin_Type);
		std::cout << "「" << skl.Name << "」 ";
		for (auto& ptr : skl.Coin)
		{
			if (ptr.Type == "Unbreakable")
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
		std::cout << " 攻击等级： " << skl.Attack_Level << "\n";
		std::cout << "基础威力： " << skl.Base << " "
			<< "变动威力： " << skl.Change << " "
			<< "攻击类型： " << attacktype_to_string.find(skl.Attack_Type)->second << " " << "罪孽类型： ";;
		setSinColor(skl.Sin_Type);
		std::cout << sintype_to_string.find(skl.Sin_Type)->second << "\n";
		setColor(8);
	}

	void showEffectStatus(const BattleCharacter& ch) {
		if (ch.breath.x != 0 && ch.breath.y != 0)
		{
			setColor(15);
			std::cout << "呼吸法:[" << ch.breath.x << "][" << ch.breath.y << "] ";
		}
		if (ch.burn.x != 0 && ch.burn.y != 0)
		{
			setColor(12);
			std::cout << "烧伤:[" << ch.burn.x << "][" << ch.burn.y << "] ";
		}
		if (ch.bleed.x != 0 && ch.bleed.y != 0)
		{
			setColor(4);
			std::cout << "流血:[" << ch.bleed.x << "][" << ch.bleed.y << "] ";
		}
		if (ch.rupture.x != 0 && ch.rupture.y != 0)
		{
			setColor(10);
			std::cout << "破裂:[" << ch.rupture.x << "][" << ch.rupture.y << "] ";
		}
		if (ch.sink.x != 0 && ch.sink.y != 0)
		{
			setColor(1);
			std::cout << "沉沦:[" << ch.sink.x << "][" << ch.sink.y << "] ";
		}
		if (ch.tremor.x != 0 && ch.tremor.y != 0)
		{
			setColor(14);
			std::cout << "震颤:[" << ch.tremor.x << "][" << ch.tremor.y << "]";
		}
		std::cout << "\n\n";
		setColor(8);
	}

private:
	BattleCharacter Player;
	BattleCharacter Enemy;
	State state = State::input;
	int round;
	int choice;
	std::array<int, 2> dashboard;

	ActionQueue action_queue;
};