#pragma once
#include <iostream>
#include <queue>
#include "Character.h"
#include "EventBus.h"

#pragma execution_character_set("utf-8")

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
	int* total_damage;
	float* mul_1;
	float* mul_2;
	float* add_1;
	float* add_2;
};

class ActionQueue
{
public:
	ActionQueue() = default;
	~ActionQueue() = default;

	void addAction(ActionType type, BattleCharacter& a, BattleCharacter& b, float priority) {
		//std::cout << "【日志】 " << "调用ActionQueue::addAction" << "\n";
		//std::cout << (int)type << " " << a.Data->Name << " " << b.Data->Name << " " << priority << "\n";
		Action action;
		action.action_type = type;
		action.priority = priority;
		action.a = &a;
		action.b = &b;
		queue.push(action);
	}

	bool isEmpty() const {
		return queue.empty();
	}

	void executeAction() {
		Action action = queue.top();
		BattleCharacter* winner = nullptr;
		BattleCharacter* loser = nullptr;
		switch (action.action_type)
		{
		case ActionType::Combat:
			// 拼点
			makeCombat(*action.a, *action.b);

			// 玩家点数大
			if (action.a->skill.Point > action.b->skill.Point) { winner = action.a; loser = action.b; }
			// 敌人点数大
			else { winner = action.b; loser = action.a; }

			// 增减理智
			winner->addSanity(static_cast<int>(winner->Data->Sanity.y * 0.2));
			loser->addSanity(-static_cast<int>(loser->Data->Sanity.y * 0.1));

			// 为胜者加一个单方面攻击
			addAction(ActionType::Unilateral, *winner, *loser, action.priority - 0.1f);

			// 若败者有红币，则加一个单方面攻击
			if (loser->skill.isRedCoinContained())
			{
				addAction(ActionType::Unilateral, *loser, *winner, action.priority - 0.2f);
			}
			break;
		case ActionType::Unilateral:
			makeDamage(*action.a, *action.b);
			break;
		default:
			break;
		}
		setColor(8);
		queue.pop();
	}

public:

	bool isLevelDiff(BattleCharacter& ch_0, BattleCharacter& ch_1) {
		return ch_0.skill.Attack_Level > ch_1.skill.Attack_Level;
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

		DamageEventData damage_event_data;

		do
		{
			do
			{

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

				// 打印a的技能硬币
				setSinColor(a.skill.Sin_Type);
				std::cout << "「" << a.skill.Name << "」";
				printEachCoin(a.skill);
				std::cout << " " << a.skill.Point << " ";
				// 打印b的技能硬币
				setSinColor(b.skill.Sin_Type);
				std::cout << "「" << b.skill.Name << "」";
				printEachCoin(b.skill);
				std::cout << " " << b.skill.Point << "\n\n";

				Sleep(150);

			} while (a.skill.Point == b.skill.Point);	// 拼点直到点数不同

			// 若a点数大于b, 摧毁b最前的硬币, 否则摧毁a最前的硬币
			if (a.skill.Point > b.skill.Point) { b.skill.destoryFrontCoin(); }
			else { a.skill.destoryFrontCoin(); }

		} while (!a.skill.isAllCoinBroke() && !b.skill.isAllCoinBroke());

		// 若硬币破碎，变动值固定为1
		if (a.skill.isAllCoinBroke()) { a.skill.Change = 1; }
		if (b.skill.isAllCoinBroke()) { b.skill.Change = 1; }
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
				std::cout << round(ch.skill.Coin[i].Damage);
				if (i != ch.skill.Coin.size() - 1)
				{
					std::cout << "+";
				}
			}
		}
	}

	void makeDamage(BattleCharacter& attacker, BattleCharacter& target) {
		initNum(attacker);
		
		setColor(15);
		std::cout << "[日志] " << attacker.Data->Name << " 对 " << target.Data->Name << "使用了 ";
		setSinColor(attacker.skill.Sin_Type);
		std::cout << "「" << attacker.skill.Name << "」 \n";
		setColor(8);

		// 计算等压
		float diff = attacker.skill.Attack_Level - target.skill.Attack_Level;

		// 抗性
		float resist = target.Data->Attack_Type_Resist.at(attacker.skill.Attack_Type);
		if (target.status == Status::confusion) { resist = 2.0f; }

		// 总伤害
		int total_damage = 0;

		// 增伤
		float mul_1 = 1;	// 第一类乘算增伤
		float mul_2 = 1;	// 第二类乘算增伤
		float add_1 = 0;	// 第一类加算增伤
		float add_2 = 0;	// 第二类加算增伤

		DamageEventData data;
		data.attacker = &attacker;
		data.target = &target;
		data.mul_1 = &mul_1;
		data.mul_2 = &mul_2;
		data.add_1 = &add_1;
		data.add_2 = &add_2;
		data.total_damage = &total_damage;

		//遍历
		for (size_t i = 0; i < attacker.skill.Coin.size(); i++)
		{
			data.coin = &attacker.skill.Coin[i];

			mul_1 = 1;
			mul_2 = 1;
			add_1 = 0;
			add_2 = 0;

			// 广播事件 伤害前
			EventBus::get().dispatch(BattleEvent::BeforeDamage, &data);

			if (!attacker.skill.Coin[i].is_Broke || attacker.skill.Coin[i].Type == "Unbreakable")
			{
				// 投掷
				attacker.rollCoinPoint(i);
				EventBus::get().dispatch(BattleEvent::rollCoin, &data);

				// 目前币为正面
				if (attacker.skill.Coin[i].current_Face)
				{
					// 目前总点数
					attacker.skill.Point += attacker.skill.Change;
					// 目前币点数
					attacker.skill.Coin[i].Point += attacker.skill.Point;
				}

				// 计算本次伤害
				attacker.skill.Coin[i].Damage = attacker.skill.Point;

				// 当前硬币造成的伤害=当前硬币数值×第一类乘算增伤×第二类乘算增伤+第一类加算增伤+第二类加算增伤
				// 攻防差值
				mul_1 += diff / (abs(diff) + 25);
				// 暴击率
				if (attacker.handleBreath()) { mul_1 += 0.2; EventBus::get().dispatch(BattleEvent::Critical, &data); }
				// 抗性
				attacker.skill.Coin[i].Damage += resist;
				// 乘以第一类乘算增伤
				attacker.skill.Coin[i].Damage *= mul_1;
				// 乘以第二类乘算增伤
				attacker.skill.Coin[i].Damage *= mul_2;
				// 加以第一类加算增伤
				attacker.skill.Coin[i].Damage += add_1;
				// 加以第二类加算增伤
				attacker.skill.Coin[i].Damage += add_2;

				// 检查伤害至少为1
				if (attacker.skill.Coin[i].Damage <= 1) { attacker.skill.Coin[i].Damage = 1; }

				// 减少目标本硬币伤害的血量
				target.health -= round(attacker.skill.Coin[i].Damage);

				// 把当前币伤害加入总伤害
				total_damage += round(attacker.skill.Coin[i].Damage);

				EventBus::get().dispatch(BattleEvent::Damage, &data);
			}

			// 广播事件 伤害后
			EventBus::get().dispatch(BattleEvent::AfterDamage, &data);

			std::cout << "\n";
			Sleep(150);
		}

		printEachCoin(attacker.skill);
		printEachDamage(attacker);

		std::cout << "\n[日志] 技能攻击部分最终造成了 " << total_damage << " 点伤害。" << "\n\n";
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