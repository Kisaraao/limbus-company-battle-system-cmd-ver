#include <iostream>
#include <Windows.h>

#include "pugixml.hpp"
#include "Vector2.h"
#include "Type.h"
#include "ConsoleUtils.h"
#include "Random Manager.h"
#include "EventBus.h"
#include "Data.h"
#include "Effect.h"
#include "Character.h"
#include "ActionSlot.h"
#include "XML.h"
#include "Character Pool.h"
#include "Battle Manager.h"

bool Running = true;

static void leader(const std::string& path, CharacterPool& pool, std::string& player, std::string& enemy, int* player_slot, int* enemy_slot) {
	pugi::xml_document doc;
	doc.load_file(path.c_str());

	auto node = doc.child("Leader").children("Character");
	for (auto& ptr : node)
	{
		pool.addCharacter(ptr.attribute("path").as_string());
	}

	player = doc.child("Leader").child("Battle").child("Player").attribute("name").as_string();
	enemy = doc.child("Leader").child("Battle").child("Enemy").attribute("name").as_string();

	*player_slot = doc.child("Leader").child("Battle").child("Player").attribute("slots").as_int();
	*enemy_slot = doc.child("Leader").child("Battle").child("Enemy").attribute("slots").as_int();
}

int main() {
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	CharacterPool character_pool;
	std::string player;
	std::string enemy;

	int player_slot;
	int enemy_slot;

	leader("script/Leader.xml", character_pool, player, enemy, &player_slot, &enemy_slot);

	AttackType::get().loadXML("script/Type.xml");
	SinType::get().loadXML("script/Type.xml");

	BattleManager battle_manager(character_pool.get(player), character_pool.get(enemy), player_slot, enemy_slot);

	battle_manager.on_enter();
	while (Running)
	{
		battle_manager.on_update();
		battle_manager.on_draw();
	}
	battle_manager.on_exit();

	system("pause");

	return 0;
}