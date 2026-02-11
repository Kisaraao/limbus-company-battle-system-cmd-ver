#pragma once
#include <iostream>
#include <Windows.h>
#include "Type.h"

void setColor(int color) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
}

void setSinColor(const SinType& sintype) {
	switch (sintype)
	{
	case SinType::None:
		setColor(8);
		break;
	case SinType::Pride:
		setColor(1);
		break;
	case SinType::Wrath:
		setColor(4);
		break;
	case SinType::Lust:
		setColor(12);
		break;
	case SinType::Sloth:
		setColor(6);
		break;
	case SinType::Gluttony:
		setColor(10);
		break;
	case SinType::Envy:
		setColor(5);
		break;
	case SinType::Melancholy:
		setColor(3);
		break;
	default:
		setColor(15);
		break;
	}
}