#pragma once
#include <iostream>

#include "Effect.h"

class SingleCountEffect : public StatusEffect
{
public:
	SingleCountEffect() = default;
	~SingleCountEffect() = default;

	virtual void gain(int val) = 0;

};