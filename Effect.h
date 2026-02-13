#pragma once
#include <iostream>
#include "Vector2.h"
#include "EventBus.h"

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

//class Effect
//{
//public:
//	std::string type;
//	virtual void gain(const Vector2& vec) {};
//	virtual int onActive() {};
//	virtual int onExplode() {};
//};
//
//class Burn : public Effect
//{
//public:
//	Burn() {
//		this->type = "Burn";
//	}
//public:
//	Vector2 val = { 0,0 };
//	void gain(const Vector2& vec) override {
//		val += vec;
//		checkOver(val);
//		keepAble(val);
//	}
//	int onActive() override {
//		int damage = 0;
//		if (val.y != 0)
//		{
//			--val.y;
//			damage = val.x;
//			if (val.y == 0)
//			{
//				val.x = 0;
//			}
//		}
//		return damage;
//	};
//	int onExplode() override {
//		int damage = 0;
//		if (val.x != 0 && val.y != 0)
//		{
//			damage = round(val.y * val.x);
//			val = { 0,0 };
//		}
//		return damage;
//	}
//};
//
//class Bleed : public Effect
//{
//public:
//	Bleed() {
//		this->type = "Bleed";
//	}
//public:
//	Vector2 val = { 0,0 };
//	void gain(const Vector2& vec) override {
//		val += vec;
//		checkOver(val);
//		keepAble(val);
//	}
//	int onActive() override {
//		int damage = 0;
//		if (val.y != 0)
//		{
//			--val.y;
//			damage = val.x;
//			if (val.y == 0)
//			{
//				val.x = 0;
//			}
//		}
//		return damage;
//	};
//	int onExplode() override {}
//};

//struct Effect
//{
//	std::string type;
//	Vector2 val;
//};