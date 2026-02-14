#pragma once
#include <iostream>
#include <memory>
#include "pugixml.hpp"
#include "Character.h"
#include "Vector2.h"

#pragma execution_character_set("utf-8")

class CharacterXML
{
public:
	void loadXML(const std::string& path, CharacterTemplate* ch) {
		pugi::xml_document doc;
		doc.load_file(path.c_str());
		auto node = doc.child("Character");
		// 角色名称
		ch->name = node.child("Name").attribute("val").as_string();
		// 特性关键词
		for (pugi::xml_node& tag : node.children("Tag")) { ch->tag_list.push_back(tag.attribute("val").as_string()); }
		// 等级
		ch->level = node.child("Level").attribute("val").as_int();
		// 血量上下限
		ch->health = Vector2(node.child("Health").attribute("Min").as_int(), node.child("Health").attribute("Max").as_int());
		// 开局血量
		ch->begin_health = node.child("Health").text().as_int(ch->health.y);
		// 理智上下限
		ch->sanity = Vector2(node.child("Sanity").attribute("Min").as_int(), node.child("Sanity").attribute("Max").as_int());
		// 开局理智
		ch->begin_sanity = node.child("Sanity").text().as_int(0);
		// 开局速度
		ch->speed = Vector2(node.child("Speed").attribute("Min").as_int(), node.child("Speed").attribute("Max").as_int());
		// 抗性
		for (auto& r : node.children("Resist")) { ch->resist[r.attribute("name").as_string()] = r.attribute("val").as_float(); }
		// 混乱阈值队列
		for (pugi::xml_node& c : node.children("Confusion")) { ch->confusion.push(c.attribute("val").as_float()); }
		// 技能列表
		for (pugi::xml_node& skl : node.children("Skill"))
		{
			Skill skill;
			// 权重
			ch->skill_weight.push_back(skl.child("Count").attribute("val").as_int());
			// 类型
			skill.type = skl.attribute("type").as_string("attack");
			// 名称
			skill.name = skl.child("Name").attribute("val").as_string();
			// 攻击等级
			skill.attack_level = skl.child("Attack_Level").attribute("val").as_int() + ch->level;
			// 基础值
			skill.base = skl.child("Base").attribute("val").as_int();
			// 变动值
			skill.change = skl.child("Change").attribute("val").as_int();
			// 攻击类型
			skill.attack_type = skl.child("Type").attribute("attack").as_string();
			// 罪孽类型
			skill.sin_type = skl.child("Type").attribute("sin").as_string();
			// 硬币列表
			for (pugi::xml_node& c : skl.children("Coin"))
			{
				Coin coin;
				// 类型
				coin.type = c.attribute("type").as_string("Normal");
				// 特色文本
				coin.before = c.child("Before").attribute("content").as_string();
				coin.after = c.child("After").attribute("content").as_string();
				// 效果列表
				if (!c.children("Effect").empty())
				{
					for (auto& ptr : c.children("Effect"))
					{
						CoinEffect effect;
						effect.target = ptr.attribute("target").as_string();
						effect.type = ptr.attribute("type").as_string();
						effect.mode = ptr.attribute("mode").as_string();
						effect.value.x = ptr.attribute("x").as_float(0.0f);
						effect.value.y = ptr.attribute("y").as_float(0.0f);
						coin.effects.push_back(effect);
					}
				}
				skill.coin_list.push_back(coin);
			}
			// 使用时
			if (!skl.child("Using").empty())
			{
				for (auto& ptr : skl.child("Using").children("Effect"))
				{
					CoinEffect effect;
					effect.target = ptr.attribute("target").as_string();
					effect.type = ptr.attribute("type").as_string();
					effect.mode = ptr.attribute("mode").as_string();
					effect.value.x = ptr.attribute("x").as_float(0.0f);
					effect.value.y = ptr.attribute("y").as_float(0.0f);
					skill.using_list.push_back(effect);
				}
			}
			// 拼点前后
			if (!skl.child("Combat").empty())
			{
				if (!skl.child("Combat").child("Win").empty())
				{
					for (auto& ptr : skl.child("Combat").child("Win").children("Effect"))
					{
						CoinEffect win;
						win.target = ptr.attribute("target").as_string();
						win.type = ptr.attribute("type").as_string();
						win.mode = ptr.attribute("mode").as_string();
						win.value.x = ptr.attribute("x").as_float(0.0f);
						win.value.y = ptr.attribute("y").as_float(0.0f);
						skill.combat_win.push_back(win);
					}
				}
				if (!skl.child("Combat").child("Lose").empty())
				{
					for (auto& ptr : skl.child("Combat").child("Lose").children("Effect"))
					{
						CoinEffect lose;
						lose.target = ptr.attribute("target").as_string();
						lose.type = ptr.attribute("type").as_string();
						lose.mode = ptr.attribute("mode").as_string();
						lose.value.x = ptr.attribute("x").as_float(0.0f);
						lose.value.y = ptr.attribute("y").as_float(0.0f);
						skill.combat_lose.push_back(lose);
					}
				}
			}
			ch->skill_list.push_back(skill);
		}
	}
};