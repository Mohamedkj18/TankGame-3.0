
#include "RunGames.h"



TankAlgorithmFactory make_tank_factory(size_t algo_id) {
    return [algo_id](int player_index, int tank_index) {
        auto& reg = AlgorithmRegistrar::getAlgorithmRegistrar();
        auto& entry = reg.getPlayerAndAlgoFactory(algo_id);
        return entry.createTankAlgorithm(player_index, tank_index); // unique_ptr<TankAlgorithm>
    };
}


std::unique_ptr<Player> make_player(size_t algo_id,
    int player_index,
    size_t x, size_t y,
    size_t max_steps, size_t num_shells)
{
auto& reg = AlgorithmRegistrar::getAlgorithmRegistrar();
auto& entry = reg.getPlayerAndAlgoFactory(algo_id);
return entry.createPlayer(player_index, x, y, max_steps, num_shells); // unique_ptr<Player>
}


std::string satelliteViewToString(const SatelliteView& view, size_t width, size_t height) {
    std::string s;
    s.reserve((width+1)*height);
    for (size_t y=0;y<height;y++) {
        for (size_t x=0;x<width;x++) s.push_back(view.getObjectAt(x,y));
            s.push_back('\n');
    }
    std::cout << "Satellite View String: " << s << "\n";
    return s;
}




RanGame run_single_game(const GameArgs& g, bool verbose) {
    // 1) GameManager by ID
    auto& gmReg = GameManagerRegistrar::getGameManagerRegistrar();
    auto it = gmReg.gameManagers.find(g.GameManagerID);
    if (it == gmReg.gameManagers.end() || !it->second.hasFactory()) {
        throw std::runtime_error("GameManager not found or not loadable: " + g.GameManagerName);
    }
    std::unique_ptr<AbstractGameManager> gm = it->second.create(verbose);  // factory(verbose) under the hood 

    // 2) Factories for the two algorithms
    TankAlgorithmFactory f1 = make_tank_factory(g.playerAndAlgoFactory1ID);
    TankAlgorithmFactory f2 = make_tank_factory(g.playerAndAlgoFactory2ID);

    // 3) Players for each side (initial x,y are placeholders; GM/Player usually handle placement using the SatelliteView)
    std::unique_ptr<Player> p1 = make_player(g.playerAndAlgoFactory1ID, /*player_index=*/1, g.map_width, g.map_height, g.max_steps, g.num_shells);
    std::unique_ptr<Player> p2 = make_player(g.playerAndAlgoFactory2ID, /*player_index=*/2, g.map_width, g.map_height, g.max_steps, g.num_shells);

    
    for(int i = 0; i < g.map_width; ++i) {
        for(int j = 0; j < g.map_height; ++j) {
            std::cout << g.map->getObjectAt(i, j);
        }
        std::cout << "\n";
    }

    std::cout << "Running game with GameManager: " << g.GameManagerName
              << ", Map: " << g.map_name
              << ", Player 1: " << g.player1Name
              << ", Player 2: " << g.player2Name
             << ", Max Steps: " << g.max_steps << " Width: " << g.map_width << " Height: " << g.map_height <<"\n";

    GameResult res = gm->run(
        g.map_width, g.map_height,
        std::move(*g.map),                      // const SatelliteView&
        g.map_name,
        g.max_steps, g.num_shells,
        *p1, g.player1Name, *p2, g.player2Name,
        f1, f2
    );
    std::cout << "Game finished: " << "aaaaaaaaaaa" << "\n";
    std::string gameFinalState = satelliteViewToString(*res.gameState , g.map_width, g.map_height); // Assuming SatelliteView has a toString() method for final state
    return RanGame{ g.GameManagerName, g.map_name, g.playerAndAlgoFactory1ID, g.playerAndAlgoFactory2ID, std::move(res), gameFinalState };
}