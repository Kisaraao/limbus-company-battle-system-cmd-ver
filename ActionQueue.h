#pragma once
#include <iostream>
#include <queue>
#include <Windows.h>
#include <cmath>

#include "Character.h"
#include "EventBus.h"
#include "ConsoleUtils.h"
#include "ActionSlot.h"

#pragma execution_character_set("utf-8")

enum class ActionType
{
	Combat,		// 拼点
	Unilateral,	// 单方面攻击
};

struct Action
{
	ActionType action_type;
	ActionSlot* a;
	ActionSlot* b;
	float priority;  // 添加优先级字段
};

struct UsingSkillEventData {
	ActionSlot* self;
	ActionSlot* target;
};

struct CombatEventData
{
	CharacterInstance* a;
	CharacterInstance* b;
};

struct AfterCombatEventData {
	ActionSlot* winner;
	ActionSlot* loser;
};

struct DamageEventData {
	ActionSlot* attacker;
	ActionSlot* target;
	Coin* coin;
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

	void addAction(ActionType type, ActionSlot& a, ActionSlot& b, float priority) { queue.push({ type, &a, &b, priority }); }

	bool isEmpty() const { return queue.empty(); }

	void executeAction() {
		// 获取第一个行动
		Action action = queue.top();
		queue.pop();
		// 赢者与败者
		ActionSlot* winner = nullptr;
		ActionSlot* loser = nullptr;

		UsingSkillEventData data = { action.a, action.b };

		AfterCombatEventData aftercombat;

		switch (action.action_type)
		{
		case ActionType::Combat:
			// 广播 使用时
			EventBus::get().dispatch(BattleEvent::UsingSkill, &data);
			// 广播 使用时
			data = { action.b, action.a };
			EventBus::get().dispatch(BattleEvent::UsingSkill, &data);

			// 广播 拼点前
			EventBus::get().dispatch(BattleEvent::BeforeCombat, &action);

			// 拼点
			makeCombat(*action.a, *action.b);

			// 玩家点数大
			if (action.a->selecting.total_point > action.b->selecting.total_point) { winner = action.a; loser = action.b; }
			// 敌人点数大
			else { winner = action.b; loser = action.a; }

			// 广播 拼点后
			aftercombat.winner = winner;
			aftercombat.loser = loser;
			EventBus::get().dispatch(BattleEvent::AfterCombat, &aftercombat);

			// 增减理智
			winner->Owner->addSanity(static_cast<int>(winner->Owner->Data->sanity.y * 0.2));
			loser->Owner->addSanity(-static_cast<int>(winner->Owner->Data->sanity.y * 0.1));

			// 为胜者加一个单方面攻击
			addAction(ActionType::Unilateral, *winner, *loser, action.priority - 0.1f);

			// 若败者有红币，则加一个单方面攻击
			if (loser->selecting.isRedCoinContained())
			{
				addAction(ActionType::Unilateral, *loser, *winner, action.priority - 0.2f);
			}
			break;
		case ActionType::Unilateral:
			if (!action.a->Owner->isConfused())
			{
				makeDamage(*action.a, *action.b);
			}
			break;
		default:
			break;
		}
		setColor(8);
	}

public:

	bool isLevelDiff(Skill& a, Skill& b) {
		return a.attack_level > b.attack_level;
	}

	void initNum(ActionSlot& slot) {
		// 初始化coin的Point, Damage
		for (auto& ptr : slot.selecting.coin_list)
		{
			ptr.point = 0;
			ptr.damage = 0;
		}
		slot.selecting.total_point = 0;
		slot.selecting.total_damage = 0;
	}

	void rollEachCoin(ActionSlot& slot) {
		// 遍历
		for (auto& ptr : slot.selecting.coin_list)
		{
			ptr.roll(slot.Owner->sanity);
		}
	}

	void rollCoin(ActionSlot& slot, int num) {
		slot.selecting.coin_list[num].roll(slot.Owner->sanity);
	}

	void printEachCoin(ActionSlot& slot) {
		// 打印硬币
		for (auto& ptr : slot.selecting.coin_list)
		{
			if (ptr.type == "Unbreakable")
			{
				if (ptr.is_Broke)
				{
					if (ptr.current_Face)//破碎红币 正面	□ 12
					{
						setColor(12);
					}
					else {//破碎红币 反面	□ 4
						setColor(8);
					}
					std::cout << "□";
				}
				else {
					if (ptr.current_Face)//红币 正面		■ 12
					{
						setColor(12);
					}
					else {//红币 反面		■ 4
						setColor(8);
					}
					std::cout << "■";
				}
			}
			else
			{
				if (ptr.is_Broke)//破碎普通硬币	× 8
				{
					setColor(8);
					std::cout << "×";
				}
				else {
					if (ptr.current_Face)//普通硬币 正面	● 14
					{
						setColor(14);
					}
					else {//普通硬币 反面	● 6
						setColor(8);
					}
					std::cout << "●";
				}
			}
			setColor(8);
		}
	}

	void printEachDamage(ActionSlot& slot) {
		// 打印伤害
		std::cout << "\n";
		for (size_t i = 0; i < slot.selecting.coin_list.size(); i++)
		{
			if (!slot.selecting.coin_list[i].is_Broke || slot.selecting.coin_list[i].type == "Unbreakable")  // 未破碎的普通硬币
			{
				std::cout << round(slot.selecting.coin_list[i].damage);
				if (i != slot.selecting.coin_list.size() - 1)
				{
					std::cout << "+";
				}
			}
		}
	}

	void makeCombat(ActionSlot& a, ActionSlot& b) {
		// 初始化每个硬币的point与damage
		initNum(a);
		initNum(b);
		// 创建拼点事件传输数据
		CombatEventData data;
		data.a = a.Owner;
		data.b = b.Owner;
		// 拼点直到 a与b的点数不同 && 有一方硬币全部被摧毁
		do
		{
			do
			{
				// 投掷a与b所有硬币
				rollEachCoin(a);
				rollEachCoin(b);
				// 广播 投掷硬币
				EventBus::get().dispatch(BattleEvent::RollCoin, &data);
				// 计算 a 点数
				a.selecting.total_point = a.selecting.base;
				// 等压
				if (isLevelDiff(a.selecting, b.selecting)) { a.selecting.total_point += static_cast<int>((a.selecting.attack_level - b.selecting.attack_level) / 3); }
				// 根据硬币正面次数增加点数
				for (auto& ptr : a.selecting.coin_list)
				{
					if (!ptr.is_Broke && ptr.current_Face)
					{
						a.selecting.total_point += a.selecting.change;
					}
				}
				// 计算 b 点数
				b.selecting.total_point = b.selecting.base;
				// 等压
				if (isLevelDiff(b.selecting, a.selecting)) { b.selecting.total_point += static_cast<int>((b.selecting.attack_level - a.selecting.attack_level) / 3); }
				// 根据硬币正面次数增加点数
				for (auto& ptr : b.selecting.coin_list)
				{
					if (!ptr.is_Broke && ptr.current_Face)
					{
						b.selecting.total_point += b.selecting.change;
					}
				}

				// 打印a的技能硬币
				setSinColor(a.selecting.sin_type);
				std::cout << "「" << a.selecting.name << "」";
				printEachCoin(a);
				std::cout << " " << a.selecting.total_point << " ";
				// 打印b的技能硬币
				setSinColor(b.selecting.sin_type);
				std::cout << "「" << b.selecting.name << "」";
				printEachCoin(b);
				std::cout << " " << b.selecting.total_point << "\n";

				Sleep(100);
			} while (a.selecting.total_point == b.selecting.total_point);

			// 若a点数大于b, 摧毁b最前的硬币；若b点数大于a，摧毁a最前的硬币
			if (a.selecting.total_point > b.selecting.total_point) { b.selecting.destoryFrontCoin(); }
			else if (b.selecting.total_point > a.selecting.total_point) { a.selecting.destoryFrontCoin(); }

		} while (!a.selecting.isAllCoinBroke() && !b.selecting.isAllCoinBroke());

		// 若所有硬币破碎，变动值固定为1
		if (a.selecting.isAllCoinBroke()) { a.selecting.change = 0; }
		if (b.selecting.isAllCoinBroke()) { b.selecting.change = 0; }

		std::cout << "\n";
	}

	void makeDamage(ActionSlot& attacker, ActionSlot& target) {
		initNum(attacker);
		initNum(target);
		
		setColor(15);
		std::cout << "[日志] " << attacker.Owner->Data->name << " 对 " << target.Owner->Data->name << " 使用了 ";
		setSinColor(attacker.selecting.sin_type);
		std::cout << "「" << attacker.selecting.name << "」 \n\n";
		setColor(8);

		// 计算等压
		float diff = attacker.selecting.attack_level - target.selecting.attack_level;
		// 增伤
		float mul_1 = 1;	// 第一类乘算增伤
		float mul_2 = 1;	// 第二类乘算增伤
		float add_1 = 0;	// 第一类加算增伤
		float add_2 = 0;	// 第二类加算增伤
		// 创建伤害事件传输数据
		DamageEventData data;
		data.attacker = &attacker;
		data.target = &target;
		data.mul_1 = &mul_1;
		data.mul_2 = &mul_2;
		data.add_1 = &add_1;
		data.add_2 = &add_2;
		Coin* current = nullptr;
		// 设初始点数为基础值
		attacker.selecting.total_point = attacker.selecting.base;

		// 遍历每个硬币
		for (size_t i = 0; i < attacker.selecting.coin_list.size(); i++)
		{
			data.coin = &attacker.selecting.coin_list[i];
			current = &attacker.selecting.coin_list[i];
			// 重置增伤数值
			mul_1 = 1;
			mul_2 = 1;
			add_1 = 0;
			add_2 = 0;

			if (!current->is_Broke || current->type == "Unbreakable")
			{
				std::cout << "[硬币] " << i + 1 << ":\n";

				// 抗性
				float resist = target.Owner->Data->resist.at(attacker.selecting.attack_type);
				if (target.Owner->isConfused()) { resist = 2.0f; }
				if (resist < 1.0f)
				{
					resist = -resist;
				}

				// 广播事件 伤害前
				EventBus::get().dispatch(BattleEvent::BeforeDamage, &data);

				// 投掷
				rollCoin(attacker, i);

				// 目前币为正面
				if (current->current_Face)
				{
					// 目前总点数
					attacker.selecting.total_point += attacker.selecting.change;
					// 目前币点数
					current->point += attacker.selecting.total_point;
				}

				// 计算本次伤害
				current->damage = attacker.selecting.total_point;

				// 当前硬币造成的伤害=当前硬币数值×第一类乘算增伤×第二类乘算增伤+第一类加算增伤+第二类加算增伤
				// 乘以第一类乘算增伤
				// 攻防差值
				mul_1 += diff / (abs(diff) + 25);
				// 暴击率
				auto critical = Effect::active::breath(attacker.Owner->breath);
				if (critical.has_value()) { if (critical.value()) { mul_1 += 0.2; EventBus::get().dispatch(BattleEvent::Critical, &data); } }
				// 抗性
				mul_1 += resist;
				std::cout << "[日志] 抗性：";
				if (resist > 1) { setColor(6); std::cout << "脆弱"; }
				else if (resist < 1) { std::cout << "抵抗"; }
				else { setColor(7); std::cout << "一般"; }
				std::cout << " x" << resist << "\n";
				setColor(8);
				std::cout << "[日志] 第一类乘算增伤：";
				setColor(15);
				std::cout << "x" << (int)(mul_1 * 100) << "%\n";
				setColor(8);
				current->damage *= mul_1;

				// 乘以第二类乘算增伤
				
				// 伤害强化
				if (attacker.Owner->damage_enhance.x > 0.0f)
				{
					float damage_enhance = min(attacker.Owner->damage_enhance.x / 10.0f, 1.0f);
					mul_2 += damage_enhance;
				}
				// 伤害弱化
				if (attacker.Owner->damage_weak.x > 0.0f)
				{
					float damage_weak = min(attacker.Owner->damage_weak.x / 10.0f, 1.0f);
					mul_2 -= damage_weak;
				}
				// 守护
				if (target.Owner->protect.x > 0.0f)
				{
					float protect = min(target.Owner->protect.x / 10.0f, 1.0f);
					mul_2 -= protect;
				}

				std::cout << "[日志] 第二类乘算增伤：";
				setColor(15);
				std::cout << "x" << (int)(mul_2 * 100) << "%\n";
				setColor(8);
				current->damage *= mul_2;

				// 加以第一类加算增伤
				current->damage += add_1;
				// 加以第二类加算增伤
				current->damage += add_2;

				// 检查伤害至少为1
				if (current->damage < 1) { current->damage = 1; }

				// 减少目标本硬币伤害的血量
				target.Owner->addHealth(-round(current->damage));

				// 把当前币伤害加入总伤害
				attacker.selecting.total_damage += round(current->damage);

				EventBus::get().dispatch(BattleEvent::Damage, &data);

				std::cout << "[日志] 该硬币造成了 ";
				setColor(15);
				std::cout << round(current->damage);
				setColor(8);
				std::cout << " 点伤害。" << "\n";

				// 广播事件 伤害后
				EventBus::get().dispatch(BattleEvent::AfterDamage, &data);

				std::cout << "\n";
			}

			Sleep(100);
		}

		printEachCoin(attacker);
		printEachDamage(attacker);

		std::cout << "\n[日志] 该技能最终造成了 ";
		setColor(15);
		std::cout << attacker.selecting.total_damage;
		setColor(8);
		std::cout << " 点伤害。" << "\n\n";
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