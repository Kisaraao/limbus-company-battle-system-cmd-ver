#pragma once
#include <iostream>
#include <optional>
#include <algorithm>

#include "Vector2.h"

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

namespace Effect {	
	void keepValid(Vector2& vec) {
		vec.x = std::clamp(static_cast<int>(vec.x), 0, 99);
		vec.y = std::clamp(static_cast<int>(vec.y), 0, 99);

		if (vec.x == 0 && vec.y != 0) vec.x = 1;
		if (vec.y == 0 && vec.x != 0) vec.y = 1;
	}

	void add(Vector2& vec, const Vector2& num) {
		vec += num;
		keepValid(vec);
	}

	namespace active {
		// 触发dot类型特效伤害，注意返回的是正数
		std::optional<int> tick(Vector2& vec) {
			if (vec == Vector2(0, 0)) { return std::nullopt; }
			int damage = vec.x;
			--vec.y;
			if (vec.y == 0) { vec = { 0, 0 }; }
			return damage;
		}

		std::optional<bool> breath(Vector2& vec) {
			if (vec == Vector2(0, 0)) { return std::nullopt; }
			if (RandomManager::get().probability(min(vec.x * 0.05, 1)))
			{
				--vec.y;
				return true;
			}
			else { return false; }
		}
	}

	namespace burst {
		std::optional<int> dot(Vector2& vec) {
			if (vec == Vector2(0, 0)) { return std::nullopt; }
			int damage = vec.x * vec.y;
			vec = { 0, 0 };
			return damage;
		}

		std::optional<Vector2> sink(Vector2& vec, int current, const Vector2& Sanity) {
			if (vec == Vector2(0, 0)) { return std::nullopt; }
			int damage_sanity = vec.x * vec.y;
			int damage = 0;
			int available_sanity = current - Sanity.x;
			if (damage_sanity > available_sanity)
			{
				damage = round(damage_sanity - available_sanity);
				damage_sanity = available_sanity;
			}
			if (vec.y == 0) { vec = { 0, 0 }; }
			return Vector2(damage, damage_sanity);
		}

		std::optional<Vector2> tremor(Vector2& vec, const std::queue<float>& confusion) {
			if (vec == Vector2(0, 0)) { return std::nullopt; }
			int damage = 0;
			int damage_confusion = 0;
			--vec.y;
			if (confusion.empty()) { damage = round(vec.x / 2); }
			else { damage_confusion = vec.x; }
			if (vec.y == 0) { vec = { 0, 0 }; }
			return Vector2(damage, damage_confusion);
		}
	}
}