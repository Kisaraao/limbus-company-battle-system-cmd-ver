#pragma once
#include <iostream>
#include <vector>
#include "pugixml.hpp"

struct Attack_Type
{
	std::string name;
	float resist;
};

class AttackType
{
public:

	static AttackType& get() {
		static AttackType instance;
		return instance;
	}

	void loadXML(const std::string& path) {
		pugi::xml_document doc;
		doc.load_file(path.c_str());
		auto node = doc.child("Type").children("Attack");
		for (auto& ptr : node)
		{
			Attack_Type type;
			type.name = ptr.attribute("name").as_string();
			type.resist = 2.0f;
			attack_type.push_back(type);
		}
	}

	std::vector<Attack_Type> attack_type;

private:
};

struct Sin_Type
{
	std::string name;
	int color;
};

class SinType
{
public:

	static SinType& get() {
		static SinType instance;
		return instance;
	}

	void loadXML(const std::string& path) {
		pugi::xml_document doc;
		doc.load_file(path.c_str());
		auto node = doc.child("Type").children("Sin");
		for (auto& ptr : node)
		{
			Sin_Type type;
			type.name = ptr.attribute("name").as_string("нч");
			type.color = ptr.attribute("color").as_int(15);
			sin_type.push_back(type);
		}
	}

	int getColor(const std::string& name) {
		for (auto& sin : sin_type) {
			if (sin.name == name) return sin.color;
		}
		return 15;  // д╛хо╟в
	}

	std::vector<Sin_Type> sin_type;

private:
};