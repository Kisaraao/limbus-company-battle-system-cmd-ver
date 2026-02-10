#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <random>
#include <functional>
#include "Vector2.h"
#include "EventBus.h"

enum class AttackType
{
	None,
	Slash,
	Stab,
	Strike
};

std::unordered_map<std::string, AttackType> string_to_attacktype = { 
	{ u8"斩击", AttackType::Slash },
	{ u8"穿刺", AttackType::Stab },
	{ u8"打击", AttackType::Strike },
	{ u8"无", AttackType::None }
};

std::unordered_map<AttackType, std::string> attacktype_to_string = {
	{ AttackType::Slash, u8"斩击" },
	{ AttackType::Stab, u8"穿刺" },
	{ AttackType::Strike, u8"打击" },
	{ AttackType::None, u8"无" }
};

enum class SinType
{
	None,
	Pride,
	Wrath,
	Lust,
	Sloth,
	Gluttony,
	Envy,
	Melancholy
};

std::unordered_map<std::string, SinType> string_to_sintype = {
	{ u8"无", SinType::None },
	{ u8"傲慢", SinType::Pride },
	{ u8"暴怒", SinType::Wrath },
	{ u8"色欲", SinType::Lust },
	{ u8"怠惰", SinType::Sloth },
	{ u8"暴食", SinType::Gluttony },
	{ u8"嫉妒", SinType::Envy },
	{ u8"忧郁", SinType::Melancholy }
};

std::unordered_map<SinType, std::string> sintype_to_string = {
	{ SinType::None, u8"无" },
	{ SinType::Pride, u8"傲慢" },
	{ SinType::Wrath, u8"暴怒" },
	{ SinType::Lust, u8"色欲" },
	{ SinType::Sloth, u8"怠惰" },
	{ SinType::Gluttony, u8"暴食" },
	{ SinType::Envy, u8"嫉妒" },
	{ SinType::Melancholy, u8"忧郁" }
};

struct Effect
{
	std::string target; // "enemy" or "self"
	std::string type; // "bleed", "burn", "rupture", "sink"
	std::string mode; // "add", "set"
	Vector2 value; // x=强度, y=层数
};

class Coin {
public:
	//硬币类型
	std::string Type;

	bool is_Broke = false;
	bool current_Face;
	int Point;
	int Damage;

	std::vector<Effect> effects; // 效果

	//特色文本
	std::string before;
	std::string after;

public:

	void drawCoin(std::function<void(int num)> setcolor) {
		if (Type == "Unbreakable")
		{
			if (is_Broke)
			{
				if (current_Face)//破碎红币 正面	□ 12
				{
					setcolor(12);
				}
				else {//破碎红币 反面	□ 4
					setcolor(8);
				}
				std::cout << u8"□";
			}
			else {
				if (current_Face)//红币 正面		■ 12
				{
					setcolor(12);
				}
				else {//红币 反面		■ 4
					setcolor(8);
				}
				std::cout << u8"■";
			}
		}
		else
		{
			if (is_Broke)//破碎普通硬币	× 8
			{
				setcolor(8);
				std::cout << u8"×";
			}
			else {
				if (current_Face)//普通硬币 正面	● 14
				{
					setcolor(14);
				}
				else {//普通硬币 反面	● 6
					setcolor(8);
				}
				std::cout << u8"●";
			}
		}
		setcolor(8);
	}

	bool isEffectContain(const std::string& str) {
		std::cout << u8"[日志] " << u8"Coin::isEffectContain 检测 " << str << u8"是否存在\n";
		for (auto& ptr : effects)
		{
			if (ptr.type == str)
			{
				return true;
			}
		}
		return false;
	}

};

class Skill
{
public:
	Skill() = default;
	~Skill() = default;

	std::string Name;
	int Attack_Level;
	int Base;
	int Change;
	AttackType Attack_Type = AttackType::None;
	SinType Sin_Type = SinType::None;
	std::vector<Coin> Coin;

	int Point;
	float Damage;

	bool isAllCoinBroke() {
		for (size_t i = 0; i < Coin.size(); i++)
		{
			if (!Coin[i].is_Broke) {
				return false;
			}
		}
		return true;
	}

	bool isRedCoinContained() {
		for (size_t i = 0; i < Coin.size(); i++)
		{
			if (Coin[i].Type == "Unbreakable") {
				return true;
			}
		}
		return false;
	}
};

enum class Status
{
	normal,
	confusion,
	dead
};

class CharacterData
{
public:
	CharacterData(const std::string& name, const std::vector<std::string>& tag_list, const std::vector<Skill>& skill_list, int level, const Vector2& health, const Vector2& sanity, const Vector2& speed) {
		Name = name;
		Tag_List = tag_list;
		Skill_List = skill_list;
		Level = level;
		Health = health;
		Sanity = sanity;
		Speed = speed;
	}
	CharacterData() = default;
	~CharacterData() = default;

	std::string Name;
	std::vector<std::string> Tag_List;
	std::vector<Skill> Skill_List;
	std::vector<int> Skill_weight;
	int Level;
	Vector2 Health;
	Vector2 Sanity;
	Vector2 Speed;
	std::unordered_map<AttackType, float> Attack_Type_Resist;
	std::queue<float> Confusion;

private:
};

class BattleCharacter
{
public:
	BattleCharacter(const CharacterData* data) {
		Data = data;
		reset();
	}
	BattleCharacter() = default;
	~BattleCharacter() = default;

	void loadCharacterData(const CharacterData* ch) {
		Data = ch;
		reset();
	}

	void setCurrentSkill(int index) {
		skill.Name = Data->Skill_List[index].Name;
		skill.Attack_Level = Data->Skill_List[index].Attack_Level;
		skill.Base = Data->Skill_List[index].Base;
		skill.Change = Data->Skill_List[index].Change;
		skill.Attack_Type = Data->Skill_List[index].Attack_Type;
		skill.Sin_Type = Data->Skill_List[index].Sin_Type;
		skill.Coin = Data->Skill_List[index].Coin;
	}

	bool checkConfusion() {
		if (confusion.empty()) {
			return false;
		}
		if (health <= Data->Health.y * confusion.front())
		{
			//std::cout << "[事件] " << Data->Name << " 陷入了混乱！\n";
			confusion.pop();
			status = Status::confusion;
			return true;
		}
		return false;
	}

	bool checkDeath() {
		if (health <= Data->Health.x)
		{
			health = Data->Health.x;
			status = Status::dead;
			return true;
		}
		return false;
	}

	void rollSpeed() {
		speed_rd.param(std::uniform_int_distribution<int>::param_type(Data->Speed.x, Data->Speed.y));
		speed = speed_rd(random);
	}

	void rollWeight() {
		weight = weight_rd(random);
	}

	void reset() {
		if (!Data) return;
		health = Data->Health.y;
		sanity = 0;
		speed = Data->Speed.x;
		confusion = Data->Confusion;
		status = Status::normal;
		weight_rd.param(std::discrete_distribution<>::param_type(Data->Skill_weight.begin(), Data->Skill_weight.end()));
	}

public:

	void addSanity(int num) {
		sanity += num;
		if (sanity > Data->Sanity.y)
		{
			sanity = Data->Sanity.y;
		}
		else if (sanity < Data->Sanity.x)
		{
			sanity = Data->Sanity.x;
			std::cout << u8"[日志] " << Data->Name << u8"陷入了恐慌。\n";
			sanity = 0;
			addSink(Vector2(0, 10));
		}
	}

	void addHealth(int num) {
		health += num;
		if (health > Data->Health.y)
		{
			health = Data->Health.y;
		}
		else if (health < Data->Health.x)
		{
			health = Data->Health.x;
		}
	}

	bool rollCoinPoint(int index) {

		if (index >= 0 && index <= skill.Coin.size())
		{
			sanity_rd.param(std::bernoulli_distribution::param_type(0.5 + sanity * 0.01));
			if (sanity_rd(random))
			{
				skill.Coin[index].current_Face = true;
				return true;
			}
			else
			{
				skill.Coin[index].current_Face = false;
				return false;
			}
		}
		return false;
	}

public:

	std::random_device rd;
	std::mt19937 random = std::mt19937(rd());
	std::bernoulli_distribution sanity_rd;
	std::uniform_int_distribution<int> speed_rd;
	std::discrete_distribution<int> weight_rd;

	const CharacterData* Data = nullptr;

	Skill skill;

	int weight;
	int choice;
	Status status = Status::normal;
	std::queue<float> confusion;
	int health;
	int sanity;
	int speed;

public:
	// 效果

	void checkOver(Vector2& vec) {
		if (vec.x > 99)
		{
			vec.x = 99;
		}
		if (vec.x < 0)
		{
			vec.x = 0;
		}

		if (vec.y > 99)
		{
			vec.y = 99;
		}
		if (vec.y < 0)
		{
			vec.y = 0;
		}
	}

	void keepAble(Vector2& vec) {
		if (vec.x == 0)
		{
			vec.x = 1;
		}
		if (vec.y == 0)
		{
			vec.y = 1;
		}
	}

	Vector2 burn = { 0, 0 };
	void addBurn(const Vector2& vec) {
		burn += vec;
		checkOver(burn);
		keepAble(burn);
	}
	void handleBurn() {
		if (burn.y != 0)
		{
			--burn.y;
			addHealth(-burn.x);
			//health -= burn.x;
			std::cout << u8"[日志] " << Data->Name << u8" 受到 " << burn.x << u8" 点烧伤伤害" << u8" 还剩 " << burn.y << u8" 层烧伤层数。" << "\n";
			if (burn.y == 0)
			{
				burn.x = 0;
			}
		}
	}
	void burnExplode() {
		if (burn.x != 0 && burn.y != 0)
		{
			addHealth(-(burn.y * burn.x));
			std::cout << u8"[日志] " << Data->Name << u8" 被 烧伤引爆 受到 " << burn.y * burn.x << u8" 点烧伤伤害。\n";
			burn = { 0,0 };
		}
	}

	Vector2 bleed = { 0, 0 };
	void addBleed(const Vector2& vec) {
		bleed += vec;
		checkOver(bleed);
		keepAble(bleed);
	}
	void handleBleed() {
		if (bleed.y > 0)
		{
			--bleed.y;
			addHealth(-bleed.x);
			//health -= bleed.x;
			std::cout << u8"[日志] " << Data->Name << u8" 受到 " << bleed.x << u8" 点流血伤害" << u8" 还剩 " << bleed.y << u8" 层流血层数。" << "\n";
			if (bleed.y == 0)
			{
				bleed.x = 0;
			}
		}
		else
		{
			bleed = { 0,0 };
		}
	}

	Vector2 rupture = { 0, 0 };
	void addRupture(const Vector2& vec) {
		rupture += vec;
		checkOver(rupture);
		keepAble(rupture);
	}
	void handleRupture() {
		if (rupture.y > 0)
		{
			--rupture.y;
			addHealth(-rupture.x);
			//health -= rupture.x;
			std::cout << u8"[日志] " << Data->Name << u8" 受到 " << rupture.x << u8" 点破裂伤害" << u8" 还剩 " << rupture.y << u8" 层破裂层数。" << "\n";
			if (rupture.y == 0)
			{
				rupture.x = 0;
			}
		}
		else
		{
			rupture = { 0,0 };
		}
	}

	Vector2 sink = { 0, 0 };
	void addSink(const Vector2& vec) {
		sink += vec;
		checkOver(sink);
		keepAble(sink);
	}
	void handleSink() {
		if (sink.y > 0)
		{
			--sink.y;
			addSanity(-sink.x);
			std::cout << u8"[日志] " << Data->Name << u8" 受到 " << sink.x << u8" 点沉沦理智降低" << u8" 还剩 " << sink.y << u8" 层沉沦层数。" << "\n";
			if (sink.y == 0)
			{
				sink.x = 0;
			}
		}
		else
		{
			sink = { 0,0 };
		}
	}

	Vector2 tremor = { 0, 0 };
	void addTremor(const Vector2& vec) {
		tremor += vec;
		checkOver(tremor);
		keepAble(tremor);
	}
	void handleTremor() {
		if (tremor.y > 0)
		{
			--tremor.y;
			if (confusion.empty())
			{
				addHealth(-(tremor.x / 2));
				std::cout << u8"[日志] " << Data->Name << u8"被 震颤引爆 受到 " << (tremor.x / 2) << u8" 点震颤爆发伤害" << u8" 还剩 " << tremor.y << u8" 层震颤层数。" << "\n";
			}
			else {
				confusion.front() += static_cast<float>(tremor.x / Data->Health.y);
				std::cout << u8"[日志] " << Data->Name << u8"被 震颤引爆 受到 " << tremor.x << u8" 点混乱阈值前移" << u8" 还剩 " << tremor.y << u8" 层震颤层数。" << "\n";
			}
			if (tremor.y == 0)
			{
				tremor.x = 0;
			}
		}
		else
		{
			tremor = { 0,0 };
		}
	}
};