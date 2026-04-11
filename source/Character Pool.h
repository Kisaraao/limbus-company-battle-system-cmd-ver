#pragma once
#include <iostream>
#include <memory>

#include "Character.h"
#include "XML.h"

class CharacterPool
{
public:
	CharacterPool() = default;
	~CharacterPool() {
		for (auto& pair : character_pool) {
			pair.second = nullptr;
		}
		character_pool.clear();
	}

	bool addCharacter(const std::string& name) {
		if (character_pool.find(name) != character_pool.end()) {
			return false; // 角色已存在
		}

		std::string path = "script/Character/" + name + ".xml";
		std::unique_ptr<CharacterTemplate> ch = std::make_unique<CharacterTemplate>();
		xml.loadXML(path, ch.get());
		character_pool[name] = std::move(ch);
	}

	CharacterTemplate* get(const std::string& name) {
		auto it = character_pool.find(name);
		if (it != character_pool.end()) {
			return it->second.get();
		}
		return nullptr; // 明确返回空指针
	}

private:
	std::unordered_map<std::string, std::unique_ptr<CharacterTemplate>> character_pool;
	CharacterXML xml;
};