#include <iostream>

#include <boost/dll.hpp>

#include <sol/sol.hpp>
#include <sol/types.hpp>
#include <magic_enum.hpp>

#include <cpp-dump.hpp>

struct Card
{
	std::string name;
	int elixir_cost = 0;
};

struct CardHand {
	Card card_1;
	Card card_2;
	Card card_3;
	Card card_4;
};

enum Team {
	RED,
	BLUE,
};

struct Unit {
	Team team;
	int health = 0;
};

struct Arena {
	std::vector<Unit> units;
};

struct Tower {
	int health = 0;
};

struct Player {
	std::string name;
	int elixir = 0;
	CardHand card_hand;
	Tower left_tower;
	Tower right_tower;
};

struct GameState {
	CardHand card_hand;
	Card next_card;
	Player current_player;
	Player opponent_player;
	Arena arena;
};

struct Position {
	int x = 0;
	int y = 0;
};

struct ActionCard {
	std::string card_name;
	Position tile_position;
};
CPP_DUMP_DEFINE_EXPORT_OBJECT(ActionCard, card_name, tile_position);
CPP_DUMP_DEFINE_EXPORT_OBJECT(Position, x, y);

class Bot {
	std::chrono::steady_clock::time_point last_run_time;
public:
	Bot() {
		std::cout << "Bot has loaded" << std::endl;
	}

	void step(sol::this_state state, sol::function callback) {
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_run_time);
		if (elapsed.count() < 10) {
			return;
		}
		last_run_time = now;
		GameState game_state;
		sol::table state_table = get_state_table(state, game_state);
		const std::optional<ActionCard> maybe_action_card = parse_callback(callback.call(sol::nil, state_table));
		if (!maybe_action_card.has_value()) {
			return;
		}
		cpp_dump(maybe_action_card.value());
	}

private:
	sol::table get_state_table(sol::this_state state, GameState& game_state) {
		sol::state_view existing_state = state;
		sol::table state_table = existing_state.create_table();
		state_table["card_hand"] = get_card_hand_table(state, game_state.card_hand);
		state_table["next_card"] = get_card_table(state, game_state.next_card);
		state_table["current_player"] = get_player_table(state, game_state.current_player);
		state_table["opponent_player"] = get_player_table(state, game_state.opponent_player);
		state_table["arena"] = get_arena_table(state, game_state.arena);
		return state_table;
	}

	sol::table get_card_hand_table(sol::this_state state, CardHand& card_hand) {
		sol::state_view existing_state = state;
		sol::table card_hand_table = existing_state.create_table();
		card_hand_table["card_1"] = get_card_table(state, card_hand.card_1);
		card_hand_table["card_2"] = get_card_table(state, card_hand.card_2);
		card_hand_table["card_3"] = get_card_table(state, card_hand.card_3);
		card_hand_table["card_4"] = get_card_table(state, card_hand.card_4);
		return card_hand_table;
	}

	sol::table get_card_table(sol::this_state state, Card& card) {
		sol::state_view existing_state = state;
		sol::table card_table = existing_state.create_table();
		card_table["name"] = card.name;
		card_table["elixir_cost"] = card.elixir_cost;
		return card_table;
	}

	sol::table get_player_table(sol::this_state state, Player& player) {
		sol::state_view existing_state = state;
		sol::table player_table = existing_state.create_table();
		player_table["name"] = player.name;
		player_table["elixir"] = player.elixir;
		player_table["card_hand"] = get_card_hand_table(state, player.card_hand);
		player_table["left_tower"] = get_tower_table(state, player.left_tower);
		player_table["right_tower"] = get_tower_table(state, player.right_tower);
		return player_table;
	}

	sol::table get_tower_table(sol::this_state state, Tower& tower) {
		sol::state_view existing_state = state;
		sol::table tower_table = existing_state.create_table();
		tower_table["health"] = tower.health;
		return tower_table;
	}

	sol::table get_arena_table(sol::this_state state, Arena& arena) {
		sol::state_view existing_state = state;
		sol::table arena_table = existing_state.create_table();
		sol::table units_table = existing_state.create_table();
		for (int i = 0; i < arena.units.size(); i++) {
			units_table[i + 1] = get_unit_table(state, arena.units[i]);
		}
		arena_table["units"] = units_table;
		return arena_table;
	}

	sol::table get_unit_table(sol::this_state state, Unit& unit) {
		sol::state_view existing_state = state;
		sol::table unit_table = existing_state.create_table();
		unit_table["team"] = magic_enum::enum_name(unit.team);
		unit_table["health"] = unit.health;
		return unit_table;
	}

	std::optional<ActionCard> parse_callback(const sol::unsafe_function_result& callback_result) {
		if (callback_result.get_type() != sol::type::table) {
			return std::nullopt;
		}
		sol::table action_card_table = callback_result.get<sol::table>();
		ActionCard action_card;
		action_card.card_name = action_card_table.get_or<std::string>("card_name", "");

		Position tile_position = [&]() {
			if (!action_card_table.get<sol::table>("tile_position").valid()) {
				return Position{ 0, 0 };
			}
			sol::table tile_position_table = action_card_table.get<sol::table>("tile_position");
			Position tile_position;
			tile_position.x = tile_position_table.get_or("x", 0);
			tile_position.y = tile_position_table.get_or("y", 0);
			return tile_position;
			}
		();
		action_card.tile_position = tile_position;
		return action_card;
	}
};

int main() {
	auto main_lua = boost::dll::program_location().parent_path() / "main.lua";

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::coroutine);
	lua.set_function("tick", []() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}
	);

	auto bot = lua["cr"].get_or_create<sol::table>();
	bot.new_usertype<Bot>("Bot",
		"step", &Bot::step
	);

	lua.script_file(main_lua.string());

	return 0;
}