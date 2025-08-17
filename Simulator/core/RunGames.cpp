
#include "RunGames.h"



static TankAlgorithmFactory make_tank_factory(size_t algo_id) {
    return [algo_id](int player_index, int tank_index) {
        auto& reg = AlgorithmRegistrar::getAlgorithmRegistrar();
        auto& entry = reg.getPlayerAndAlgoFactory(algo_id);
        return entry.createTankAlgorithm(player_index, tank_index); // unique_ptr<TankAlgorithm>
    };
}


static std::unique_ptr<Player> make_player(size_t algo_id,
    int player_index,
    size_t x, size_t y,
    size_t max_steps, size_t num_shells)
{
auto& reg = AlgorithmRegistrar::getAlgorithmRegistrar();
auto& entry = reg.getPlayerAndAlgoFactory(algo_id);
return entry.createPlayer(player_index, x, y, max_steps, num_shells); // unique_ptr<Player>
}





static RanGame run_single_game(const GameArgs& g, bool verbose) {
    // 1) GameManager by name
    auto& gmReg = GameManagerRegistrar::getGameManagerRegistrar();
    auto it = gmReg.gameManagers.find(g.GameManagerID);
    if (it == gmReg.gameManagers.end() || !it->second.hasFactory()) {
        throw std::runtime_error("GameManager not found or not loadable: " + g.GameManagerName);
    }
    std::unique_ptr<AbstractGameManager> gm = it->second.create(verbose);  // factory(verbose) under the hood 

    // 2) Factories for the two algorithms
    TankAlgorithmFactory f1 = make_tank_factory(g.playerAndAlgoFactory1Name);
    TankAlgorithmFactory f2 = make_tank_factory(g.playerAndAlgoFactory2Name);

    // 3) Players for each side (initial x,y are placeholders; GM/Player usually handle placement using the SatelliteView)
    std::unique_ptr<Player> p1 = make_player(g.playerAndAlgoFactory1Name, /*player_index=*/1, g.map_width, g.map_height, g.max_steps, g.num_shells);
    std::unique_ptr<Player> p2 = make_player(g.playerAndAlgoFactory2Name, /*player_index=*/2, g.map_width, g.map_height, g.max_steps, g.num_shells);

    // 4) Run one game
    GameResult res = gm->run(
        g.map_width, g.map_height,
        *g.map,                      // const SatelliteView&
        g.map_name,
        g.max_steps, g.num_shells,
        *p1, g.player1Name, *p2, g.player2Name,
        f1, f2
    );

    return RanGame{ g.GameManagerName, g.map_name, g.playerAndAlgoFactory1Name, g.playerAndAlgoFactory2Name, std::move(res) };
}
