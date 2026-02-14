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
	void moveFrontConfusion(float val) {
		confusion.front() += static_cast<float>(val / Data->health.y);
		if (confusion.front() > Data->health.y) { confusion.front() = 1.0f; }
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
				std::cout << "[日志] " << Data->name;
				setColor(6);
				std::cout << " 陷入混乱！\n";
				setColor(8);
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
			std::cout << "[日志] " << Data->name << "陷入了恐慌。\n";
			sanity = 0;
			Effect::add(sink, Vector2(0, 5));
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
	// 效果
	Vector2 burn = { 0, 0 };
	Vector2 bleed = { 0, 0 };
	Vector2 rupture = { 0, 0 };
	Vector2 sink = { 0, 0 };
	Vector2 tremor = { 0, 0 };
	Vector2 breath = { 0,0 };
	// 状态
	Vector2 attack_level_up = { 0,0 };
	Vector2 attack_level_down = { 0,0 };
	Vector2 strong = { 0,0 };
	Vector2 weak = { 0,0 };
};