#pragma once
#include <iostream>
#include <queue>
#include "Character.h"
#include "EventBus.h"

enum class ActionType
{
	Combat,		//拼点
	Unilateral,	//单方面攻击
};

struct Action
{
	ActionType action_type;
	BattleCharacter* a;
	BattleCharacter* b;
	float priority;  // 添加优先级字段
};

struct DamageEventData {
	BattleCharacter* attacker;
	BattleCharacter* target;
	Coin* coin;
};

class ActionQueue
{
public:
	ActionQueue() = default;
	~ActionQueue() = default;

	void addAction(ActionType type, BattleCharacter& a, BattleCharacter& b, float priority) {
		//std::cout << u8"【日志】 " << u8"调用ActionQueue::addAction" << "\n";
		//std::cout << (int)type << " " << a.Data->Name << " " << b.Data->Name << " " << priority << "\n";
		Action action;
		action.action_type = type;
		action.priority = priority;
		action.a = &a;
		action.b = &b;
		queue.push(action);
	}

	bool isEmpty() const {
		//std::cout << u8"【日志】 " << u8"调用ActionQueue::isEmpty: " << queue.empty() << "\n";
		return queue.empty();
	}

	void executeAction() {
		//std::cout << u8"【日志】 " << u8"调用ActionQueue::executeAction" << "\n";

		Action action = queue.top();

		//std::cout << u8"【日志】 " << u8"当前type: " << (int)action.action_type << "\n";
		//std::cout << u8"【日志】 " << u8"当前priority: " << action.priority << "\n";

		switch (action.action_type)
		{
		case ActionType::Combat:
			
			makeCombat(*action.a, *action.b);
			if (action.a->skill.Point > action.b->skill.Point)		//玩家点数大
			{
				action.a->addSanity((int)(action.a->Data->Sanity.y * 0.2));
				action.b->addSanity((int)-(action.b->Data->Sanity.y * 0.1));
				//makeDamage(*action.a, *action.b);
				//拼点胜利，就加一个单方面攻击
				addAction(ActionType::Unilateral, *action.a, *action.b, action.priority - 0.1f);
				if (action.b->skill.isRedCoinContained())	//红币反击
				{
					//makeDamage(*action.b, *action.a);
					addAction(ActionType::Unilateral, *action.b, *action.a, action.priority - 0.2f);
				}
			}
			else {	//敌人点数大
				action.a->addSanity((int)-(action.a->Data->Sanity.y * 0.1));
				action.b->addSanity((int)(action.b->Data->Sanity.y * 0.2));
				//makeDamage(*action.b, *action.a);
				addAction(ActionType::Unilateral, *action.b, *action.a, action.priority - 0.1f);
				if (action.a->skill.isRedCoinContained())	//红币反击
				{
					//makeDamage(*action.a, *action.b);
					addAction(ActionType::Unilateral, *action.a, *action.b, action.priority - 0.2f);
				}
			}
			

			//setColor(15);

			//std::cout << u8"【拼点】 "
			//	<< action.a->Data->Name << u8" 的 " << action.a->skill.Name << u8" 与 "
			//	<< action.b->Data->Name << u8" 的 " << action.b->skill.Name << "\n";

			setColor(8);

			break;
		case ActionType::Unilateral:
			
			makeDamage(*action.a, *action.b);

			break;
		default:
			break;
		}

		queue.pop();
	}

public:

	bool isLevelDiff(BattleCharacter& ch_0, BattleCharacter& ch_1) {
		if (ch_0.skill.Attack_Level > ch_1.skill.Attack_Level)
		{
			return true;
		}
		return false;
	}

	void initNum(BattleCharacter& ch) {
		//初始化coin的Point, Damage
		for (auto& ptr : ch.skill.Coin)
		{
			ptr.Point = 0;
			ptr.Damage = 0;
		}
		//初始化skill的point, damage
		ch.skill.Damage = 0;
		ch.skill.Point = ch.skill.Base;
	}

	void makeCombat(BattleCharacter& a, BattleCharacter& b) {
		initNum(a);
		initNum(b);

		std::cout << "\n";

		do
		{
			do
			{
				DamageEventData damage_event_data;

				//遍历 a
				rollEachCoin(a);
				damage_event_data.attacker = &a;
				damage_event_data.target = &b;
				EventBus::get().dispatch(BattleEvent::rollCoin, &damage_event_data);

				//遍历 b
				rollEachCoin(b);
				damage_event_data.attacker = &b;
				damage_event_data.target = &a;
				EventBus::get().dispatch(BattleEvent::rollCoin, &damage_event_data);

				//计算 a 点数
				a.skill.Point = a.skill.Base;
				if (isLevelDiff(a, b))
				{
					a.skill.Point += static_cast<int>((a.skill.Attack_Level - b.skill.Attack_Level) / 3);
				}
				for (auto& ptr : a.skill.Coin)
				{
					if (!ptr.is_Broke)
					{
						if (ptr.current_Face)
						{
							a.skill.Point += a.skill.Change;
							ptr.Point = a.skill.Point;
						}
					}
				}

				//计算 b 点数
				b.skill.Point = b.skill.Base;
				//等压
				if (isLevelDiff(b, a))
				{
					b.skill.Point += static_cast<int>((b.skill.Attack_Level - a.skill.Attack_Level) / 3);
				}
				for (auto& ptr : b.skill.Coin)
				{
					if (!ptr.is_Broke)
					{
						if (ptr.current_Face)
						{
							b.skill.Point += b.skill.Change;
							ptr.Point = b.skill.Point;
						}
					}
				}

				//打印a的技能硬币
				setSinColor(a.skill.Sin_Type);
				std::cout << "「" << a.skill.Name << "」 ： ";
				printEachCoin(a);
				std::cout << " " << a.skill.Point << "\n";

				//打印b的技能硬币
				setSinColor(b.skill.Sin_Type);
				std::cout << "「" << b.skill.Name << "」 ： ";
				printEachCoin(b);
				std::cout << " " << b.skill.Point << "\n\n";

				Sleep(150);

			} while (a.skill.Point == b.skill.Point);

			//若a点数大于b, 摧毁b最前的硬币
			if (a.skill.Point > b.skill.Point)
			{
				for (auto& ptr : b.skill.Coin) {
					if (!ptr.is_Broke)
					{
						ptr.is_Broke = true;
						break;
					}
				}
			}
			//若b点数大于a, 摧毁a最前的硬币
			else
			{
				for (auto& ptr : a.skill.Coin) {
					if (!ptr.is_Broke)
					{
						ptr.is_Broke = true;
						break;
					}
				}
			}

		} while (!a.skill.isAllCoinBroke() && !b.skill.isAllCoinBroke());
	}

	void rollEachCoin(BattleCharacter& ch) {
		//遍历
		for (size_t i = 0; i < ch.skill.Coin.size(); i++)
		{
			if (!ch.skill.Coin[i].is_Broke)  // 未破碎的普通硬币
			{
				ch.rollCoinPoint(i);
			}
		}
	}

	void printEachCoin(BattleCharacter& ch) {
		//打印硬币
		for (auto& ptr : ch.skill.Coin)
		{
			ptr.drawCoin([&](int color) {
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, color);
				});
		}
	}
	void printEachCoin(Skill& skill) {
		//打印硬币
		for (auto& ptr : skill.Coin)
		{
			ptr.drawCoin([&](int color) {
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, color);
				});
		}
	}

	void printEachDamage(BattleCharacter& ch) {
		//打印伤害
		std::cout << "\n";
		for (size_t i = 0; i < ch.skill.Coin.size(); i++)
		{
			if (!ch.skill.Coin[i].is_Broke || ch.skill.Coin[i].Type == "Unbreakable")  // 未破碎的普通硬币
			{
				std::cout << ch.skill.Coin[i].Damage;
				if (i != ch.skill.Coin.size() - 1)
				{
					std::cout << u8"+";
				}
			}
		}
	}

	void makeDamage(BattleCharacter& attacker, BattleCharacter& target) {
		setColor(15);
		std::cout << u8"【" << attacker.Data->Name << u8"】 ";
		setColor(8);
		std::cout << u8"对 ";
		setColor(15);
		std::cout << u8"【" << target.Data->Name << u8"】 ";
		setColor(8);
		std::cout << u8"使用了 ";
		setSinColor(attacker.skill.Sin_Type);
		std::cout << u8"「" << attacker.skill.Name << u8"」 \n";
		setColor(8);

		initNum(attacker);
		////等压
		//if (isLevelDiff(attacker, target))
		//{
		//	attacker.skill.Point += static_cast<int>((attacker.skill.Attack_Level - target.skill.Attack_Level) / 3);
		//	//std::cout << u8"等压： " << static_cast<int>((attacker.skill.Attack_Level - target.skill.Attack_Level) / 3) << "\n";
		//}
		float diff = attacker.skill.Attack_Level - target.skill.Attack_Level;
		//抗性
		float resist = target.Data->Attack_Type_Resist.at(attacker.skill.Attack_Type);
		if (target.status == Status::confusion)
		{
			resist = 2.0f;
		}
		//std::cout << u8"抗性： " << resist << "\n";

		DamageEventData damage_event_data;

		damage_event_data.attacker = &attacker;
		damage_event_data.target = &target;

		//遍历
		for (size_t i = 0; i < attacker.skill.Coin.size(); i++)
		{

			//特色文本 before
			if (!attacker.skill.Coin[i].before.empty())
			{
				setColor(15);
				std::cout << u8"【" << attacker.Data->Name << u8"】 ";
				setColor(8);
				std::cout << u8"： ";
				setColor(15);
				std::cout << u8"“" << attacker.skill.Coin[i].before << u8"”\n";
				setColor(8);
			}

			if (!attacker.skill.Coin[i].is_Broke)  // 未破碎
			{
				//投掷
				attacker.rollCoinPoint(i);
				EventBus::get().dispatch(BattleEvent::rollCoin, &damage_event_data);
				
				//正面
				if (attacker.skill.Coin[i].current_Face)
				{
					//total point
					attacker.skill.Point += attacker.skill.Change;
					//point
					attacker.skill.Coin[i].Point += attacker.skill.Point;
				}
				//damage
				attacker.skill.Coin[i].Damage = static_cast<int>(attacker.skill.Point * resist);
				// 等压
				if (isLevelDiff(attacker, target))
				{
					// 总伤害=伤害值*[1+攻防能力差值/（|攻防能力差值|+25）]
					int oldDamage = attacker.skill.Coin[i].Damage;
					attacker.skill.Coin[i].Damage = static_cast<int>(attacker.skill.Coin[i].Damage * (1 + diff / (abs(diff) + 10)));
					//std::cout << u8"[等压] 攻击等级" << attacker.skill.Attack_Level
					//	<< u8" vs 防御等级" << target.skill.Attack_Level
					//	<< u8" 差值=" << diff
					//	<< u8" 原伤害=" << oldDamage
					//	<< u8" 增幅后=" << attacker.skill.Coin[i].Damage << "\n";
				}
				if (attacker.skill.Coin[i].Damage <= 1)
				{
					attacker.skill.Coin[i].Damage = 1;
				}
				target.health -= attacker.skill.Coin[i].Damage;
				//total damage
				attacker.skill.Damage += attacker.skill.Coin[i].Damage;

				damage_event_data.coin = &attacker.skill.Coin[i];
				EventBus::get().dispatch(BattleEvent::Damage, &damage_event_data);
			}
			else if (attacker.skill.Coin[i].Type == "Unbreakable")	//红币
			{
				attacker.rollCoinPoint(i);
			
				attacker.handleBleed();
				
				//正面
				if (attacker.skill.Coin[i].current_Face)
				{
					//total point
					attacker.skill.Point += 1;
					//point
					attacker.skill.Coin[i].Point += attacker.skill.Point;
				}
				//damage
				attacker.skill.Coin[i].Damage = static_cast<int>(attacker.skill.Point * resist);
				if (attacker.skill.Coin[i].Damage <= 1)
				{
					attacker.skill.Coin[i].Damage = 1;
				}
				target.health -= attacker.skill.Coin[i].Damage;
				//total damage
				attacker.skill.Damage += attacker.skill.Coin[i].Damage;

				damage_event_data.coin = &attacker.skill.Coin[i];
				EventBus::get().dispatch(BattleEvent::Damage, &damage_event_data);
			}

			//特色文本 before
			if (!attacker.skill.Coin[i].after.empty())
			{
				setColor(15);
				std::cout << u8"【" << attacker.Data->Name << u8"】 ";
				setColor(8);
				std::cout << u8"： ";
				setColor(15);
				std::cout << u8"“" << attacker.skill.Coin[i].after << u8"”\n";
				setColor(8);
			}
		}

		printEachCoin(attacker);
		printEachDamage(attacker);

		std::cout << "\n";
		//target.health -= attacker.skill.Damage;

		std::cout << u8"最终造成了 " << attacker.skill.Damage << u8" 点伤害。" << "\n\n";
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
	// 比较函数对象，用于优先级队列
	struct CompareAction {
		bool operator()(const Action& a, const Action& b) const {
			// 值大的优先级高（最大堆）
			return a.priority < b.priority;
		}
	};

	// priority_queue 模板参数：<元素类型, 底层容器类型, 比较类型>
	std::priority_queue<Action, std::vector<Action>, CompareAction> queue;
};