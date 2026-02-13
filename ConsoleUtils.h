#pragma once
#include <iostream>
#include <Windows.h>
#include "Type.h"

void setColor(int color) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
}

void setSinColor(const std::string& type) {
	for (auto& ptr : SinType::get().sin_type)
	{
		if (type == ptr.name)
		{
			setColor(ptr.color);
		}
	}
}