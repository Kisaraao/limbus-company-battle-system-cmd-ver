#pragma once
#include <iostream>

#include "Effect.h"

class DoubleCountEffect : public StatusEffect
{
public:
	DoubleCountEffect() {
		potency_min = 0;
		potency_max = 99;
		potency = 0;
		count_min = 0;
		count_max = 99;
		count = 0;
	}
	~DoubleCountEffect() = default;

	virtual void gain(Vector2& vec) = 0;

	// 层数上下限
	int count_max;
	int count_min;
	// 目前层数数值
	int count;

};

class Burn : public DoubleCountEffect
{
public:
	Burn() = default;
	~Burn() = default;

	void gain(Vector2& vec) override {
		potency += vec.x;
		count += vec.y;
		potency = limit(potency);
		count = limit(count);
	}

	void active() override {

	}

};