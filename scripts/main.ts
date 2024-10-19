import { types } from "./types";

const bot = new types.Bot();
bot.run((state: types.GameState) => {
    return {
        card_name: "",
        tile_position: { x: 1, y: 2 }
    }
})
