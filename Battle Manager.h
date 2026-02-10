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

		EventBus::get().subscribe(BattleEvent::TurnEnd, [this](void* data) {
			// 这个lambda就是监听器
			//std::cout << u8"[事件] 回合结束了！\n";
			Player.handleBurn();
			Enemy.handleBurn();
			});

		EventBus::get().subscribe(BattleEvent::Damage, [this](void* data) {
			//std::cout << u8"[事件] 造成伤害！\n";

			DamageEventData* Data = static_cast<DamageEventData*>(data);
			BattleCharacter* target = nullptr;

			Data->target->handleRupture();
			Data->target->handleSink();

			if (!Data->coin->effects.empty())
			{
				//std::cout << u8"[效果] 硬币有效果施加。\n";
				for (auto& ptr : Data->coin->effects) {

					if (ptr.target == "enemy")
					{
						target = Data->target;
					}
					else if (ptr.target == "self")
					{
						target = Data->attacker;
					}

					if (ptr.type == "burn")
					{
						target->addBurn(ptr.value);
						std::cout << u8"[效果] 施加烧伤：" << target->Data->Name << u8" -> "
							<< u8" 强度：" << ptr.value.x << u8" 层数：" << ptr.value.y << "\n";
						//std::cout << u8"[日志] " << Data->target->Data->Name << u8" 目前烧伤强度：" << Data->target->burn.x << u8" 层数：" << Data->target->burn.y << "\n";
					}
					if (ptr.type == "bleed")
					{
						target->addBleed(ptr.value);
						std::cout << u8"[效果] 施加流血：" << target->Data->Name << u8" -> "
							<< u8" 强度：" << ptr.value.x << u8" 层数：" << ptr.value.y << "\n";
						//std::cout << u8"[日志] " << Data->target->Data->Name << u8" 目前流血强度：" << Data->target->bleed.x << u8" 层数：" << Data->target->bleed.y << "\n";
					}
					if (ptr.type == "rupture")
					{
						target->addRupture(ptr.value);
						std::cout << u8"[效果] 施加破裂：" << target->Data->Name << u8" -> "
							<< u8" 强度：" << ptr.value.x << u8" 层数：" << ptr.value.y << "\n";
					}
					if (ptr.type == "sink")
					{
						target->addSink(ptr.value);
						std::cout << u8"[效果] 施加沉沦：" << target->Data->Name << u8" -> "
							<< u8" 强度：" << ptr.value.x << u8" 层数：" << ptr.value.y << "\n";
					}
					if (ptr.type == "tremor")
					{
						target->addTremor(ptr.value);
						std::cout << u8"[效果] 施加震颤：" << target->Data->Name << u8" -> "
							<< u8" 强度：" << ptr.value.x << u8" 层数：" << ptr.value.y << "\n";
					}
					if (ptr.type == "sanity")
					{
						target->addSanity(ptr.value.x);
						std::cout << u8"[效果] 理智加值：" << target->Data->Name << u8" -> " << ptr.value.x << "\n";
					}
					if (ptr.type == "health")
					{
						target->addHealth(ptr.value.x);
						std::cout << u8"[效果] 血量加值：" << target->Data->Name << u8" -> " << ptr.value.x << "\n";
					}

					if (ptr.type == "tremor-explode")
					{
						std::cout << u8"[效果] 震颤引爆：" << target->Data->Name << "\n";
						target->handleTremor();
					}
					if (ptr.type == "burn-explode")
					{
						std::cout << u8"[效果] 烧伤引爆：" << target->Data->Name << "\n";
						target->burnExplode();
					}
				}
			}
			});

		EventBus::get().subscribe(BattleEvent::rollCoin, [this](void* data) {
			// 这个lambda就是监听器
			//std::cout << u8"[事件] 投掷硬币！\n";
			//action_queue.queue.top().b->addBurn();
			//Player.handleBleed();
			//Enemy.handleBleed();
			DamageEventData* Data = static_cast<DamageEventData*>(data);
			Data->attacker->handleBleed();
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

			std::cout << u8"回合数： " << round + 1 << "\n\n";
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
				std::cout << Enemy.Data->Name << u8" 正在使用： \n";
				showSkillStatus(Enemy.skill);
			}

			if (Player.status != Status::confusion)
			{
				std::cout << u8"\n——————————————仪表盘——————————————" << "\n\n";
				for (size_t i = 0; i < dashboard.size(); i++)
				{
					setColor(15);
					std::cout << i + 1 << u8". ";
					showSkillStatus(Player.Data->Skill_List[dashboard[i]]);
					std::cout << "\n";
				}
				std::cout << u8"———————————————————————————————" << "\n";
				std::cout << u8"请选择技能： ";
				std::cin >> choice;
				choice--;
				Player.choice = dashboard[choice];
				Player.setCurrentSkill(Player.choice);
			}
			break;
		case BattleManager::State::combat:
			/*if (Player.status != Status::confusion && Enemy.status != Status::confusion)
			{
				makeCombat(Player, Enemy);

				if (Player.skill.Point > Enemy.skill.Point)		//玩家点数大
				{
					Player.addSanity(6);
					Enemy.addSanity(-3);
					makeDamage(Player, Enemy);
					if (Enemy.skill.isRedCoinContained())
					{
						makeDamage(Enemy, Player);
					}
				}
				else {	//敌人点数大
					Player.addSanity(-3);
					Enemy.addSanity(6);
					makeDamage(Enemy, Player);
					if (Player.skill.isRedCoinContained())
					{
						makeDamage(Player, Enemy);
					}
				}
			}
			else
			{
				if (Player.status == Status::confusion)		//玩家混乱
				{
					Player.addSanity(-3);
					Enemy.addSanity(6);
					makeDamage(Enemy, Player);
				}
				else	//敌人混乱
				{
					Player.addSanity(6);
					Enemy.addSanity(-3);
					makeDamage(Player, Enemy);
				}
			}*/

			//重复执行队列最前的action，然后pop掉，直到队列为空

			if (Player.status != Status::confusion && Enemy.status != Status::confusion)
			{
				action_queue.addAction(ActionType::Combat, Player, Enemy, 1);
			}
			else if (Player.status == Status::confusion)		//玩家混乱
			{
				action_queue.addAction(ActionType::Unilateral, Enemy, Player, 1);
			}
			else	//敌人混乱
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
					std::cout << u8"[日志]" << Player.Data->Name << u8" 死了！" << "\n";
				}
				else
				{
					std::cout << u8"[日志]" << Enemy.Data->Name << u8" 死了！" << "\n";
				}
				setColor(8);
				Running = false;
			}
			state = State::calc;
			break;
		case BattleManager::State::calc:
			//std::cout << u8"【日志】 " << u8"BattleManager::State::calc:on_exit" << "\n";
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
		std::cout << u8"等级： ";
		setColor(15);
		std::cout << ch.Data->Level << " ";
		setColor(8);
		std::cout << u8"当前血量： ";
		setColor(15);
		std::cout << ch.health << " / " << ch.Data->Health.y << " ";
		setColor(8);
		std::cout << u8"当前理智： ";
		if (ch.sanity < 0)
		{
			setColor(4);
		}else
		{
			setColor(9);
		}
		std::cout << ch.sanity << " ";
		setColor(8);
		std::cout << u8"当前速度： " << ch.speed << "\n";
		std::cout << u8"特性关键词： ";
		for (size_t i = 0; i < ch.Data->Tag_List.size(); i++)
		{
			std::cout << "[" << ch.Data->Tag_List[i] << "] ";
		}
		std::cout << u8"下一次混乱： ";
		if (!ch.confusion.empty()) {
			std::cout << ch.Data->Health.y * ch.confusion.front();
		}else {
			std::cout << u8"无";
		}
		if (ch.status == Status::confusion)
		{
			setColor(4);
			std::cout << u8"\n！陷入混乱！";
			setColor(8);
		}
		std::cout << "\n";
	}

	void showSkillStatus(const Skill& skl) {
		setSinColor(skl.Sin_Type);
		std::cout << u8"「" << skl.Name << u8"」 ";
		for (auto& ptr : skl.Coin)
		{
			if (ptr.Type == "Unbreakable")
			{
				setColor(12);
				std::cout << u8"■";
			}
			else
			{
				setColor(14);
				std::cout << u8"●";
			}
			setColor(8);
		}
		setColor(8);
		std::cout << u8" 攻击等级： " << skl.Attack_Level << "\n";
		std::cout << u8"基础威力： " << skl.Base << " "
			<< u8"变动威力： " << skl.Change << " "
			<< u8"攻击类型： " << attacktype_to_string.find(skl.Attack_Type)->second << " " << u8"罪孽类型： ";;
		setSinColor(skl.Sin_Type);
		std::cout << sintype_to_string.find(skl.Sin_Type)->second << "\n";
		setColor(8);
	}

	void showEffectStatus(const BattleCharacter& ch) {
		if (ch.burn.x != 0 && ch.burn.y != 0)
		{
			setColor(12);
			std::cout << u8"烧伤:[" << ch.burn.x << u8"][" << ch.burn.y << "] ";
		}
		if (ch.bleed.x != 0 && ch.bleed.y != 0)
		{
			setColor(4);
			std::cout << u8"流血:[" << ch.bleed.x << u8"][" << ch.bleed.y << "] ";
		}
		if (ch.rupture.x != 0 && ch.rupture.y != 0)
		{
			setColor(10);
			std::cout << u8"破裂:[" << ch.rupture.x << u8"][" << ch.rupture.y << "] ";
		}
		if (ch.sink.x != 0 && ch.sink.y != 0)
		{
			setColor(1);
			std::cout << u8"沉沦:[" << ch.sink.x << u8"][" << ch.sink.y << "] ";
		}
		if (ch.tremor.x != 0 && ch.tremor.y != 0)
		{
			setColor(14);
			std::cout << u8"震颤:[" << ch.tremor.x << u8"][" << ch.tremor.y << "]";
		}
		std::cout << "\n\n";
		setColor(8);
	}

	void setColor(int color) {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, color);
	}

	void setSinColor(const SinType& sintype) {
		switch (sintype)
		{
		case SinType::None:
			setColor(8);
			break;
		case SinType::Pride:
			setColor(1);
			break;
		case SinType::Wrath:
			setColor(4);
			break;
		case SinType::Lust:
			setColor(12);
			break;
		case SinType::Sloth:
			setColor(6);
			break;
		case SinType::Gluttony:
			setColor(10);
			break;
		case SinType::Envy:
			setColor(5);
			break;
		case SinType::Melancholy:
			setColor(3);
			break;
		default:
			setColor(15);
			break;
		}
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