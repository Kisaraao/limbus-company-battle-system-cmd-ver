#pragma once
#include <iostream>
#include <memory>
#include <vector>

#include "Data.h"
#include "Effect.h"
#include "Random Manager.h"

#pragma execution_character_set("utf-8")

enum class Status
{
	Normal,
	Confusion,
	Dead
};

class CharacterInstance
{
public:
	CharacterInstance(const CharacterTemplate* data) 
		: Data(data) {
		init();
	}
	CharacterInstance() = default;
	~CharacterInstance() = default;
public:
	// 原角色数据
	const CharacterTemplate* Data = nullptr;
	// 目前状态
	Status status;
	// 目前血量
	int health;
	// 目前理智
	int sanity;
	// 目前速度
	int speed;
	// 目前混乱阈值队列
	std::queue<float> confusion;
	// 已混乱的回合
	int confusion_round;
public:
	void loadData(const CharacterTemplate* ch) { Data = ch; }
	void init() {
		status = Status::Normal;
		health = Data->begin_health;
		sanity = Data->begin_sanity;
		speed = RandomManager::get().range(Data->speed.x, Data->speed.y);
		confusion = Data->confusion;
		confusion_round = 0;
	}
	void handleConfusion() {
		if (status == Status::Confusion)
		{
			confusion_round++;
			if (confusion_round == 2)
			{
				confusion_round = 0;
				status = Status::Normal;
			}
		}
	}
	bool checkConfusion() {
		if (confusion.empty()) {
			return false;
		}
		if (health <= Data->health.y * confusion.front())
		{
			if (status != Status::Confusion && status != Status::Dead)
			{
				status = Status::Confusion;
			}
			confusion.pop();
			return true;
		}
		return false;
	}
	bool checkDeath() {
		if (health <= Data->health.x)
		{
			health = Data->health.x;
			status = Status::Dead;
			return true;
		}
		return false;
	}
	void addSanity(int num) {
		sanity += num;
		if (sanity > Data->sanity.y)
		{
			sanity = Data->sanity.y;
		}
		else if (sanity < Data->sanity.x)
		{
			sanity = Data->sanity.x;
			//std::cout << "[日志] " << Data->name << "陷入了恐慌。\n";
			//sanity = 0;
			//addSink(Vector2(0, 10));
		}
	}
	void addHealth(int num) {
		health += num;
		if (health > Data->health.y)
		{
			health = Data->health.y;
		}
		else if (health < Data->health.x)
		{
			health = Data->health.x;
		}
	}
	bool isConfused() const { return status == Status::Confusion; }
public:
	// 效果 TODO: 简化
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
			std::cout << "[日志] " << Data->name << " 受到 " << burn.x << " 点烧伤伤害" << " 还剩 " << burn.y << " 层烧伤层数。" << "\n";
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
			std::cout << "[日志] " << Data->name << " 受到 " << damage << " 点烧伤伤害。\n";
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
			std::cout << "[日志] " << Data->name << " 受到 " << bleed.x << " 点流血伤害" << " 还剩 " << bleed.y << " 层流血层数。" << "\n";
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
			std::cout << "[日志] " << Data->name << " 受到 " << rupture.x << " 点破裂伤害" << " 还剩 " << rupture.y << " 层破裂层数。" << "\n";
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
			std::cout << "[日志] " << Data->name << " 受到 " << sink.x << " 点沉沦理智降低" << " 还剩 " << sink.y << " 层沉沦层数。" << "\n";
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
			if (san_damage > sanity - Data->sanity.x)
			{
				damage = round(san_damage - (sanity - Data->sanity.x));
				san_damage = sanity - Data->sanity.x;

				addSanity(-(sanity - Data->sanity.x));
				addHealth(-damage);
			}
			else
			{
				addSanity(-san_damage);
			}
			std::cout << "[日志] " << Data->name << " 受到 " << san_damage << " 点理智伤害与 " << damage << " 点的溢出转换伤害。\n";
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
				std::cout << "[日志] " << Data->name << " 受到 " << damage << " 点震颤爆发伤害" << " 还剩 " << tremor.y << " 层震颤层数。" << "\n";
				addHealth(-damage);
			}
			else {
				confusion.front() += static_cast<float>(tremor.x / Data->health.y);
				std::cout << "[日志] " << Data->name << " 受到 " << tremor.x << " 点混乱阈值前移" << " 还剩 " << tremor.y << " 层震颤层数。" << "\n";
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
			bool flag = false;
			if (breath.x * 0.05 > 1)
			{
				flag = RandomManager::get().probability(1);
			}
			else
			{
				flag = RandomManager::get().probability(breath.x * 0.05);
			}
			if (flag)
			{
				--breath.y;
				std::cout << "[日志] " << Data->name << " 造成暴击 还剩 " << breath.y << " 层呼吸法层数。" << "\n";
				return true;
			}
			else { return false; }
		}
		return false;
	}
};