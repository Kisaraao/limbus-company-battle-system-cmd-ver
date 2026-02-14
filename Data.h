#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include "Type.h"
#include "Vector2.h"
#include "Random Manager.h"

struct CoinEffect
{
	// 触发器
	std::string trigger;
	// 目标
	std::string target;
	// 类型
	std::string type;
	// 模式
	std::string mode;
	// 数值
	Vector2 value;
};

class Coin
{
public:
	// 硬币类型
	std::string type;
	// 破碎状态
	bool is_Broke = false;
	// 正反面
	bool current_Face;
	// 点数
	float point;
	// 伤害
	float damage;
	// 效果列表
	std::vector<CoinEffect> effects;
	// 特色文本
	std::string before;
	std::string after;
	// 重投次数
	int reroll_time = 0;
public:
	bool roll(int sanity) {
		current_Face = RandomManager::get().probability(0.5 + sanity * 0.01);
		return current_Face;
	}
};

class Skill
{
public:
	// 技能类型
	std::string type;
	// 技能名称
	std::string name;
	// 攻击等级
	int attack_level;
	// 基础值
	int base;
	// 变动值
	int change;
	// 攻击类型
	std::string attack_type;
	// 罪孽属性
	std::string sin_type;
	// 硬币列表
	std::vector<Coin> coin_list;
	// 使用时效果列表
	std::vector<CoinEffect> using_list;
	// 拼点胜利效果列表
	std::vector<CoinEffect> combat_win;
	// 拼点失败效果列表
	std::vector<CoinEffect> combat_lose;
	// 总点数
	int total_point;
	// 总伤害
	int total_damage;
public:
	bool isAllCoinBroke() {
		for (size_t i = 0; i < coin_list.size(); i++)
		{
			if (!coin_list[i].is_Broke) {
				return false;
			}
		}
		return true;
	}
	bool isRedCoinContained() {
		for (size_t i = 0; i < coin_list.size(); i++)
		{
			if (coin_list[i].type == "Unbreakable") {
				return true;
			}
		}
		return false;
	}
	bool destoryFrontCoin() {
		for (auto& ptr : coin_list) {
			if (!ptr.is_Broke)
			{
				ptr.is_Broke = true;
				return true;
			}
		}
		return false;
	}
};

struct CharacterTemplate
{
	// 角色名称
	std::string name;
	// 特性关键词列表
	std::vector<std::string> tag_list;
	// 技能权重列表
	std::vector<Skill> skill_list;
	// 技能权重列表
	std::vector<int> skill_weight;
	// 等级
	int level;
	// 血量上下限
	Vector2 health;
	// 开局血量
	int begin_health;
	// 理智上下限
	Vector2 sanity;
	// 开局理智
	int begin_sanity;
	// 速度上下限
	Vector2 speed;
	// 攻击类型抗性
	std::unordered_map<std::string, float> resist;  // 用名字当 key
	// 混乱阈值队列
	std::queue<float> confusion;
};