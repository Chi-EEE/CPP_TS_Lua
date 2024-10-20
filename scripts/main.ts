import { types } from "./types";

function main() {
    const coroutines = [
        coroutine.create(() => {
            const bot = new types.Bot();
            while (true) {
                bot.step((game_state: types.GameState) => {
                    return {
                        card_name: "",
                        tile_position: { x: 1, y: 1 }
                    }
                })
                coroutine.yield();
            }
        }),
        coroutine.create(() => {
            const bot = new types.Bot();
            while (true) {
                bot.step((game_state: types.GameState) => {
                    return {
                        card_name: "",
                        tile_position: { x: 2, y: 2 }
                    }
                })
                coroutine.yield();
            }
        })
    ]
    while (true) {
        for (const c of coroutines) {
            coroutine.resume(c)
        }
    }
}
main();