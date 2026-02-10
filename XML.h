#pragma once
#include <iostream>
#include <memory>
#include "pugixml.hpp"
#include "StringUtils.h"
#include "Character.h"
#include "Character Pool.h"
#include "Vector2.h"

class CharacterXML
{
public:
	CharacterXML() = default;
	~CharacterXML() = default;

	void loadXML(const std::string& path, CharacterData* ch) {

		pugi::xml_document doc;
		doc.load_file(path.c_str());

		auto node = doc.child("Character");

		ch->Name = node.child("Name").attribute("val").as_string();

		for (pugi::xml_node& tag : node.children("Tag"))
		{
			ch->Tag_List.push_back(tag.attribute("val").as_string());
		}

		ch->Level = node.child("Level").attribute("val").as_int();

		Vector2 Health;
		Health.x = node.child("Health").attribute("Min").as_int();
		Health.y = node.child("Health").attribute("Max").as_int();
		ch->Health = Health;

		Vector2 Sanity;
		Sanity.x = node.child("Sanity").attribute("Min").as_int();
		Sanity.y = node.child("Sanity").attribute("Max").as_int();
		ch->Sanity = Sanity;

		Vector2 Speed;
		Speed.x = node.child("Speed").attribute("Min").as_int();
		Speed.y = node.child("Speed").attribute("Max").as_int();
		ch->Speed = Speed;

		float slash_resist = node.child("Resist").attribute("Slash").as_float();
		float stab_resist = node.child("Resist").attribute("Stab").as_float();
		float strike_resist = node.child("Resist").attribute("Strike").as_float();
		ch->Attack_Type_Resist[AttackType::Slash] = slash_resist;
		ch->Attack_Type_Resist[AttackType::Stab] = stab_resist;
		ch->Attack_Type_Resist[AttackType::Strike] = strike_resist;

		for (pugi::xml_node& c : node.children("Confusion"))
		{
			ch->Confusion.push(c.attribute("val").as_float());
		}

		for (pugi::xml_node& skl : node.children("Skill"))
		{
			Skill skill;
			
			// 权重
			ch->Skill_weight.push_back(skl.child("Count").attribute("val").as_int());
			// 名称
			skill.Name = skl.child("Name").attribute("val").as_string();
			// 攻击等级
			skill.Attack_Level = skl.child("Attack_Level").attribute("val").as_int() + ch->Level;
			// 基础值
			skill.Base = skl.child("Base").attribute("val").as_int();
			// 变动值
			skill.Change = skl.child("Change").attribute("val").as_int();
			// 攻击类型
			skill.Attack_Type = string_to_attacktype.find(skl.child("Attack_Type").attribute("val").as_string())->second;
			// 罪孽类型
			skill.Sin_Type = string_to_sintype.find(skl.child("Sin_Type").attribute("val").as_string())->second;
			// 硬币
			for (pugi::xml_node& c : skl.children("Coin"))
			{
				Coin coin;
				if (c.attribute("type")) { coin.Type = c.attribute("type").as_string(); }
				else { coin.Type = "normal"; }

				// 特色文本
				if (c.child("Before")) { coin.before = c.child("Before").attribute("content").as_string(); }
				if (c.child("After")) { coin.after = c.child("After").attribute("content").as_string(); }

				// 效果
				if (!c.children("Effect").empty())
				{
					for (auto& ptr : c.children("Effect"))
					{

						Effect effect;

						effect.target = ptr.attribute("target").as_string();
						effect.type = ptr.attribute("type").as_string();
						effect.mode = ptr.attribute("mode").as_string();
						effect.value.x = ptr.attribute("x").as_float(0.0f);
						effect.value.y = ptr.attribute("y").as_float(0.0f);

						coin.effects.push_back(effect);
					}
				}

				skill.Coin.push_back(coin);
			}
			ch->Skill_List.push_back(skill);
		}
	}

private:
};