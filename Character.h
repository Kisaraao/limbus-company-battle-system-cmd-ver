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
#include "ConsoleUtils.h"
#include "Type.h"

#pragma execution_character_set("utf-8")

inline std::unordered_map<std::string, AttackType> string_to_attacktype = {
	{ "斩击", AttackType::Slash },
	{ "穿刺", AttackType::Stab },
	{ "打击", AttackType::Strike },
	{ "无", AttackType::None }
};

inline std::unordered_map<AttackType, std::string> attacktype_to_string = {
	{ AttackType::Slash, "斩击" },
	{ AttackType::Stab, "穿刺" },
	{ AttackType::Strike, "打击" },
	{ AttackType::None, "无" }
};

inline std::unordered_map<std::string, SinType> string_to_sintype = {
	{ "无", SinType::None },
	{ "傲慢", SinType::Pride },
	{ "暴怒", SinType::Wrath },
	{ "色欲", SinType::Lust },
	{ "怠惰", SinType::Sloth },
	{ "暴食", SinType::Gluttony },
	{ "嫉妒", SinType::Envy },
	{ "忧郁", SinType::Melancholy }
};

inline std::unordered_map<SinType, std::string> sintype_to_string = {
	{ SinType::None, "无" },
	{ SinType::Pride, "傲慢" },
	{ SinType::Wrath, "暴怒" },
	{ SinType::Lust, "色欲" },
	{ SinType::Sloth, "怠惰" },
	{ SinType::Gluttony, "暴食" },
	{ SinType::Envy, "嫉妒" },
	{ SinType::Melancholy, "忧郁" }
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
	float Point;
	float Damage;

	std::vector<Effect> effects; // 效果

	//特色文本
	std::string before;
	std::string after;

public:

	void drawCoin() {
		if (Type == "Unbreakable")
		{
			if (is_Broke)
			{
				if (current_Face)//破碎红币 正面	□ 12
				{
					setColor(12);
				}
				else {//破碎红币 反面	□ 4
					setColor(8);
				}
				std::cout << "□";
			}
			else {
				if (current_Face)//红币 正面		■ 12
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
			if (is_Broke)//破碎普通硬币	× 8
			{
				setColor(8);
				std::cout << "×";
			}
			else {
				if (current_Face)//普通硬币 正面	● 14
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

	bool isEffectContain(const std::string& str) {
		std::cout << "[日志] " << "Coin::isEffectContain 检测 " << str << "是否存在\n";
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

	bool destoryFrontCoin() {
		for (auto& ptr : Coin) {
			if (!ptr.is_Broke)
			{
				ptr.is_Broke = true;
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
		skill = Skill(Data->Skill_List[index]);
	}

	void handleConfusion() {
		if (status == Status::confusion)
		{
			confusion_round++;
			if (confusion_round == 2)
			{
				confusion_round = 0;
				status = Status::normal;
			}
		}
	}
	bool checkConfusion() {
		if (confusion.empty()) {
			return false;
		}
		if (health <= Data->Health.y * confusion.front())
		{
			std::cout << "[日志] " << Data->Name;
			setColor(6);
			std::cout << " 陷入了混乱！\n\n";
			setColor(8);
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
			std::cout << "[日志] " << Data->Name << "陷入了恐慌。\n";
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
	std::bernoulli_distribution breath_rd;
	std::uniform_int_distribution<int> speed_rd;
	std::discrete_distribution<int> weight_rd;

	const CharacterData* Data = nullptr;

	Skill skill;

	int weight;
	int choice;
	Status status = Status::normal;
	int health;
	int sanity;
	int speed;

	std::queue<float> confusion;
	int confusion_round = 0;

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
	int handleBurn() {
		int damage = 0;
		if (burn.y != 0)
		{
			--burn.y;
			addHealth(-burn.x);
			std::cout << "[日志] " << Data->Name << " 受到 " << burn.x << " 点烧伤伤害" << " 还剩 " << burn.y << " 层烧伤层数。" << "\n";
			damage = -burn.x;
			if (burn.y == 0)
			{
				burn.x = 0;
			}
		}
		return damage;
	}
	int burnExplode() {
		int damage = 0;
		if (burn.x != 0 && burn.y != 0)
		{
			damage = round(burn.y * burn.x);
			std::cout << "[日志] " << Data->Name << " 受到 " << damage << " 点烧伤伤害。\n";
			addHealth(-damage);
			burn = { 0,0 };
		}
		return damage;
	}

	Vector2 bleed = { 0, 0 };
	void addBleed(const Vector2& vec) {
		bleed += vec;
		checkOver(bleed);
		keepAble(bleed);
	}
	int handleBleed() {
		int damage = 0;
		if (bleed.y > 0)
		{
			--bleed.y;
			addHealth(-bleed.x);
			std::cout << "[日志] " << Data->Name << " 受到 " << bleed.x << " 点流血伤害" << " 还剩 " << bleed.y << " 层流血层数。" << "\n";
			damage = bleed.x;
			if (bleed.y == 0)
			{
				bleed.x = 0;
			}
		}
		else
		{
			bleed = { 0,0 };
		}
		return damage;
	}

	Vector2 rupture = { 0, 0 };
	void addRupture(const Vector2& vec) {
		rupture += vec;
		checkOver(rupture);
		keepAble(rupture);
	}
	int handleRupture() {
		int damage = 0;
		if (rupture.y > 0)
		{
			--rupture.y;
			addHealth(-rupture.x);
			std::cout << "[日志] " << Data->Name << " 受到 " << rupture.x << " 点破裂伤害" << " 还剩 " << rupture.y << " 层破裂层数。" << "\n";
			damage = rupture.x;
			if (rupture.y == 0)
			{
				rupture.x = 0;
			}
		}
		else
		{
			rupture = { 0,0 };
		}
		return damage;
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
			std::cout << "[日志] " << Data->Name << " 受到 " << sink.x << " 点沉沦理智降低" << " 还剩 " << sink.y << " 层沉沦层数。" << "\n";
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
	int sinkExplode() {
		int damage = 0;
		int san_damage = 0;
		if (sink.x != 0 && sink.y != 0)
		{
			san_damage = sink.y * sink.x;
			if (san_damage > sanity - Data->Sanity.x)
			{
				damage = round(san_damage - (sanity - Data->Sanity.x));
				san_damage = sanity - Data->Sanity.x;

				addSanity(-(sanity - Data->Sanity.x));
				addHealth(-damage);
			}
			else
			{
				addSanity(-san_damage);
			}
			std::cout << "[日志] " << Data->Name << " 受到 " << san_damage << " 点理智伤害与 " << damage << " 点的溢出转换伤害。\n";
			sink = { 0,0 };
		}
		return damage;
	}

	Vector2 tremor = { 0, 0 };
	void addTremor(const Vector2& vec) {
		tremor += vec;
		checkOver(tremor);
		keepAble(tremor);
	}
	int handleTremor() {
		int damage = 0;
		if (tremor.y > 0)
		{
			--tremor.y;
			if (confusion.empty())
			{
				damage = round(tremor.x / 2);
				std::cout << "[日志] " << Data->Name << " 受到 " << damage << " 点震颤爆发伤害" << " 还剩 " << tremor.y << " 层震颤层数。" << "\n";
				addHealth(-damage);
			}
			else {
				confusion.front() += static_cast<float>(tremor.x / Data->Health.y);
				std::cout << "[日志] " << Data->Name << " 受到 " << tremor.x << " 点混乱阈值前移" << " 还剩 " << tremor.y << " 层震颤层数。" << "\n";
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
		return damage;
	}

	Vector2 breath = { 0,0 };
	void addBreath(const Vector2& vec) {
		breath += vec;
		checkOver(breath);
		keepAble(breath);
	}
	bool handleBreath() {
		if (breath.x != 0 && breath.y != 0)
		{
			if (breath.x * 0.05 > 1)
			{
				breath_rd.param(std::bernoulli_distribution::param_type(1));
			}
			else
			{
				breath_rd.param(std::bernoulli_distribution::param_type(breath.x * 0.05));
			}
			if (breath_rd(random)) 
			{
				--breath.y;
				std::cout << "[日志] " << Data->Name << " 造成暴击 还剩 " << breath.y << " 层呼吸法层数。" << "\n";
				return true;
			}
			else { return false; }
		}
		return false;
	}
};