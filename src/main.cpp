#include <iostream>

#include <boost/dll.hpp>

#include <sol/sol.hpp>
#include <sol/types.hpp>

#include <cpp-dump.hpp>

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
public:
	Bot() {
	}

	void run(sol::function callback) {
		std::cout << "Bot is running" << std::endl;
		const std::optional<ActionCard> maybe_action_card = parse_callback(callback.call());
		if (!maybe_action_card.has_value()) {
			return;
		}
		cpp_dump(maybe_action_card.value());
	}

private:
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
	lua.open_libraries(sol::lib::base);
	auto bot = lua["cr"].get_or_create<sol::table>();
	bot.new_usertype<Bot>("Bot",
		"run", &Bot::run
	);
	lua.script_file(main_lua.string());
	return 0;
}