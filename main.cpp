#include <iostream>
#include <Windows.h>

#include "ConsoleUtils.h"
#include "Vector2.h"
#include "Character.h"
#include "Character Pool.h"
#include "XML.h"
#include "Battle Manager.h"

bool Running = true;

void leader(const std::string& path, CharacterPool& pool, std::string& player, std::string& enemy) {
	pugi::xml_document doc;
	doc.load_file(path.c_str());

	auto node = doc.child("Leader").children("Character");
	for (auto& ptr : node)
	{
		pool.addCharacter(ptr.attribute("path").as_string());
	}

	player = doc.child("Leader").child("Battle").child("Player").attribute("name").as_string();
	enemy = doc.child("Leader").child("Battle").child("Enemy").attribute("name").as_string();
}

int main() {

	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	CharacterPool character_pool;

	CharacterXML xml;

	std::string player;
	std::string enemy;

	leader("script/Leader.xml", character_pool, player, enemy);

	BattleManager battle_manager(character_pool.get(player), character_pool.get(enemy));

	while (Running)
	{
		battle_manager.on_enter();
		battle_manager.on_update();
		battle_manager.on_draw();
		battle_manager.on_exit();
	}

	system("pause");

	return 0;
}