#include <algorithm>
#include <array>
#include <cmath>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <sstream>
#include <utility>
#include "common/GameManagerRegistration.h"

#include "GameManager.h"
#include "MySatelliteView.h"

namespace
{
    using namespace std::chrono;

    std::atomic<uint64_t> g_run_counter{0};

    static std::string sanitize(std::string s)
    {
        for (char &c : s)
            if (!(std::isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.'))
                c = '_';
        if (s.size() > 64)
            s.resize(64);
        return s;
    }

    static std::string make_unique_base(std::string gm, std::string map,
                                        std::string alg1, std::string alg2)
    {
        const auto ts = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        const auto id = g_run_counter.fetch_add(1, std::memory_order_relaxed);
        std::ostringstream os;
        os << sanitize(gm) << "__" << sanitize(map) << "__"
           << sanitize(alg1) << "_vs_" << sanitize(alg2)
           << "__run" << id << "__ts" << ts;
        return os.str();
    }

}

namespace GameManager_212788293_212497127
{
    constexpr int MAX_STEPS_WITHOUT_SHELLS = 40;

    REGISTER_GAME_MANAGER(GameManager);
    // ------------------------ GameManager ------------------------

    GameManager::GameManager(bool verbose)
        : verbose(verbose), verbose_dir("verbose")
    {
        gameStep = 0;
        totalShellsRemaining = 0;
        playerTanksCount[1] = 0;
        playerTanksCount[2] = 0;
    }

    int GameManager::getWallHealth(int wallPos)
    {
        return walls[wallPos].health;
    }

    void GameManager::incrementGameStep() { gameStep++; }

    void GameManager::addTank(std::unique_ptr<Tank> tank)
    {
        tanks[bijection(tank->getX(), tank->getY())] = std::move(tank);
    }

    void GameManager::addShell(std::unique_ptr<Shell> shell)
    {
        int newPos = UC::bijection(shell->getX(), shell->getY());

        if (shellsFired.count(newPos))
        {
            shellsToRemove.insert(newPos);
        }

        shellsFired.insert(newPos);
        shells[newPos] = std::move(shell);
    }

    void GameManager::advanceShellsRecentlyFired()
    {
        std::unordered_set<int> newFired;

        for (int oldPos : shellsFired)
        {
            auto it = shells.find(oldPos);
            if (it == shells.end())
                continue;

            std::unique_ptr<Shell> &shell = it->second;
            bool didItMove = shell->moveForward();
            int newPos = bijection(shell->getX(), shell->getY());

            // Check for wall collision
            if (!didItMove)
            {
                // Shell hit a wall, damage wall and demolish shell
                shellHitAWall(newPos);
                shellsToRemove.insert(oldPos);
                continue; // Do not move shell forward
            }

            if (shells.count(newPos))
            {
                shellsToRemove.insert(newPos); // handle collision
            }

            shells[newPos] = std::move(shell); // move it in the map
            newFired.insert(newPos);
            shells.erase(oldPos);
        }

        shellsFired = std::move(newFired);
    }

    void GameManager::addMine(int x, int y)
    {
        mines.insert(bijection(x, y));
    }

    void GameManager::addWall(int x, int y)
    {
        Wall wall = {x, y, 2};
        walls[bijection(x, y)] = wall;
    }

    void GameManager::removeMine(int x)
    {
        mines.erase(x);
    }

    void GameManager::removeWall(int x)
    {
        walls.erase(x);
    }

    void GameManager::removeTank(int tankPos)
    {
        playerTanksCount[tanks[tankPos]->getPlayerId()]--;
        movesOfTanks[tanks[tankPos].get()->getTankGlobalId()] += " (killed)";
        tanks.erase(tankPos);
    }

    void GameManager::removeShell(int ShellPos)
    {
        shells.erase(ShellPos);
    }

    void GameManager::hitTank(int tankId)
    {
        if (tanks.count(tankId))
        {
            tanks[tankId]->hit();
        }
    }

    std::vector<std::string> GameManager::splitByComma(const std::string &input)
    {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = input.find(',');

        while (end != std::string::npos)
        {
            tokens.push_back(input.substr(start, end - start));
            start = end + 1;
            end = input.find(',', start);
        }
        tokens.push_back(input.substr(start));

        return tokens;
    }

    int GameManager::readMap(size_t w, size_t h, const SatelliteView &map)
    {
        width = static_cast<int>(w);
        height = static_cast<int>(h);

        int tankId1 = 0;
        int tankId2 = 0;
        int globalTankId = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                char c = map.getObjectAt(x, y);
                if (c == '#')
                {
                    addWall(x * 2, y * 2);
                }
                else if (c == '@')
                {
                    addMine(x * 2, y * 2);
                }
                else if (c == '1' || c == '2')
                {
                    int playerId = (c == '1') ? 1 : 2;
                    auto tank = std::make_unique<Tank>(x * 2, y * 2,
                                                       (playerId == 1) ? UC::DirectionsUtils::stringToDirection["L"]
                                                                       : UC::DirectionsUtils::stringToDirection["R"],
                                                       this, playerId, numShellsPerTank,
                                                       (playerId == 1) ? tankId1++ : tankId2++,
                                                       globalTankId++);

                    playerTanksCount[playerId]++;
                    totalShellsRemaining += numShellsPerTank;
                    addTank(std::move(tank));
                }
            }
            // removed erroneous ++y here
        }

        if (auto wopt = winnerByTanks())
        {
            return *wopt;
        }

        printBoard();

        totalTanks = tankId1 + tankId2;
        for (int k = 0; k < totalTanks; k++)
        {
            movesOfTanks.push_back(" ");
        }

        return -1;
    }

    void GameManager::checkForAMine(int x, int y)
    {
        int currTankPos = bijection(x, y);
        if (mines.count(currTankPos))
        {
            removeMine(bijection(x, y));
            tanksToRemove.insert(currTankPos);
        }
    }

    void GameManager::tankHitByAShell(int tankPos)
    {
        shellsToRemove.insert(tankPos);
        tanksToRemove.insert(tankPos);
    }

    void GameManager::shellHitAWall(int wallPos)
    {
        if (getWallHealth(wallPos) <= 0)
        {
            wallsToRemove.insert(wallPos);
        }
    }

    void GameManager::advanceShells()
    {
        Shell *shell;
        int newPos;
        bool didItMove;
        for (const auto &pair : shells)
        {
            shell = shells[pair.first].get();
            didItMove = shell->moveForward();
            newPos = bijection(shell->getX(), shell->getY());
            if (didItMove)
                checkForShellCollision(*shell);
            else
                shellHitAWall(newPos);
        }
        shells = std::move(secondaryShells);
        secondaryShells.clear();
    }

    void GameManager::reverseHandler(Tank &tank, ActionRequest move)
    {
        if (move == ActionRequest::MoveForward)
        {
            tank.resetReverseState();
            movesOfTanks[tank.getTankGlobalId()] = UC::to_string(tank.getLastMove()) + " (ignored)";
            tank.setLastMove(ActionRequest::DoNothing);
        }
        else if (tank.isReverseQueued())
        {
            movesOfTanks[tank.getTankGlobalId()] = UC::to_string(tank.getLastMove()) + " (ignored)";
            tank.incrementReverseCharge();
            if (tank.isReverseReady())
            {
                movesOfTanks[tank.getTankGlobalId()] = UC::to_string(tank.getLastMove());
                tank.executeReverse();
            }
        }
        else if (move == ActionRequest::MoveBackward)
        {
            tank.queueReverse();
            movesOfTanks[tank.getTankGlobalId()] = UC::to_string(tank.getLastMove()) + " (ignored)";
            tank.incrementReverseCharge();
            if (tank.isReverseReady())
            {
                movesOfTanks[tank.getTankGlobalId()] = UC::to_string(tank.getLastMove());
                tank.executeReverse();
            }
        }
        checkForAMine(tank.getX(), tank.getY());
    }

    void GameManager::advanceTank(Tank &tank)
    {
        ActionRequest move = tank.getLastMove();
        tank.resetReverseState();
        if (tank.moveForward())
            movesOfTanks[tank.getTankGlobalId()] = UC::to_string(move);
        else
            movesOfTanks[tank.getTankGlobalId()] = UC::to_string(move) + " (ignored)";
        checkForAMine(tank.getX(), tank.getY());
    }

    void GameManager::tankShootingShells(Tank &tank)
    {
        tank.resetReverseState();
        if (tank.canShoot())
        {
            tank.fire();
            totalShellsRemaining--;
            tank.incrementCantShoot();
        }
        tank.setLastMove(ActionRequest::DoNothing);
    }

    void GameManager::rotate(Tank &tank)
    {
        tank.resetReverseState();
        ActionRequest move = tank.getLastMove();
        if (move == ActionRequest::RotateLeft90)
            tank.rotateTank(-0.25);
        else if (move == ActionRequest::RotateRight90)
            tank.rotateTank(0.25);
        else if (move == ActionRequest::RotateLeft45)
            tank.rotateTank(-0.125);
        else if (move == ActionRequest::RotateRight45)
            tank.rotateTank(0.125);

        tank.setLastMove(ActionRequest::DoNothing);
    }

    void GameManager::checkForTankCollision(Tank &tank)
    {
        int currTankPos = bijection(tank.getX(), tank.getY());

        if (secondaryTanks.count(currTankPos))
        {
            movesOfTanks[secondaryTanks[currTankPos].get()->getTankGlobalId()] = UC::to_string(tank.getLastMove()) + " (killed)";
            playerTanksCount[secondaryTanks[currTankPos]->getPlayerId()]--;
            tanksToRemove.insert(currTankPos);
        }
        if (shells.count(currTankPos))
        {
            tankHitByAShell(currTankPos);
        }

        secondaryTanks[currTankPos] = std::make_unique<Tank>(std::move(tank));
    }

    void GameManager::checkForShellCollision(Shell &shell)
    {
        int shellPos = bijection(shell.getX(), shell.getY());
        if (tanks.count(shellPos))
            tankHitByAShell(shellPos);
        if (secondaryShells.count(shellPos))
            shellsToRemove.insert(shellPos);
        secondaryShells[shellPos] = std::make_unique<Shell>(shell);
    }

    void GameManager::executeTanksMoves(bool firstPass)
    {
        ActionRequest move;

        for (const auto &pair : tanks)
        {
            Tank *tank = pair.second.get();
            move = tank->getLastMove();
            if (tank->getCantShoot())
            {
                if (firstPass && move == ActionRequest::Shoot)
                    movesOfTanks[tank->getTankGlobalId()] = UC::to_string(move) + " (ignored)";
                tank->incrementCantShoot();
                if (tank->getCantShoot() == 8)
                    tank->resetCantShoot();
            }

            if (tank->isReverseQueued() || move == ActionRequest::MoveBackward)
            {
                reverseHandler(*tank, move);
            }
            else if (move == ActionRequest::MoveForward)
            {
                advanceTank(*tank);
            }
            else if (move == ActionRequest::Shoot)
            {
                movesOfTanks[tank->getTankGlobalId()] = UC::to_string(tank->getLastMove());
                tankShootingShells(*tank);
            }
            else if (move != ActionRequest::GetBattleInfo)
            {
                if (firstPass)
                    movesOfTanks[tank->getTankGlobalId()] = UC::to_string(tank->getLastMove());
                rotate(*tank);
            }

            checkForTankCollision(*tank);
        }

        tanks = std::move(secondaryTanks);
        secondaryTanks.clear();
    }

    void GameManager::executeBattleInfoRequests(Player &player1, Player &player2)
    {
        sortTanks();
        for (const auto &pair : tanks)
        {
            Tank *tank = pair.second.get();

            if (tank->getLastMove() == ActionRequest::GetBattleInfo)
            {
                TankAlgorithm *tankAlgorithm = tank->getTankAlgorithm();
                int pos = bijection(tank->getX(), tank->getY());
                MySatelliteView satelliteView(pos, tanks, shells, mines, walls);
                if (tank->getPlayerId() == 1)
                {
                    player1.updateTankWithBattleInfo(*tankAlgorithm, satelliteView);
                }
                else
                {
                    player2.updateTankWithBattleInfo(*tankAlgorithm, satelliteView);
                }
                movesOfTanks[tank->getTankGlobalId()] = UC::to_string(tank->getLastMove());
            }
        }
    }

    int GameManager::bijection(int x, int y)
    {
        return ((x + y) * (x + y + 1)) / 2 + y;
    }

    void GameManager::removeTanks()
    {
        for (int object : tanksToRemove)
        {
            removeTank(object);
        }
        tanksToRemove.clear();
    }

    void GameManager::removeShells()
    {
        for (int object : shellsToRemove)
        {
            removeShell(object);
        }
        shellsToRemove.clear();
    }

    void GameManager::removeWalls()
    {
        for (int object : wallsToRemove)
        {
            removeWall(object);
        }
        wallsToRemove.clear();
    }

    void GameManager::removeObjectsFromTheBoard()
    {
        removeTanks();
        removeShells();
        removeWalls();
    }

    std::optional<int> GameManager::winnerByTanks() const
    {
        if (playerTanksCount.at(1) == 0 && playerTanksCount.at(2) > 0)
            return 2;
        if (playerTanksCount.at(2) == 0 && playerTanksCount.at(1) > 0)
            return 1;
        if (playerTanksCount.at(1) == 0 && playerTanksCount.at(2) == 0)
            return 0;
        return std::nullopt;
    }

    void GameManager::sortTanks()
    {
        std::vector<std::pair<int, Tank *>> tankVec;

        for (auto &pair : tanks)
            tankVec.emplace_back(pair.first, pair.second.get());

        std::sort(tankVec.begin(), tankVec.end(),
                  [](const auto &a, const auto &b)
                  {
                      return a.second->getTankGlobalId() < b.second->getTankGlobalId();
                  });
    }

    GameResult GameManager::run(size_t map_width, size_t map_height,
                                const SatelliteView &map,
                                string map_name,
                                size_t max_steps, size_t num_shells,
                                Player &player1, string name1, Player &player2, string name2,
                                TankAlgorithmFactory player1_tank_algo_factory,
                                TankAlgorithmFactory player2_tank_algo_factory)
    {
        // clear any previous state
        clearGameState();

        GameResult result{};
        result.winner = 0;

        // set game-wide params
        maxSteps = static_cast<int>(max_steps);
        numShellsPerTank = static_cast<int>(num_shells);

        // step1: load the map
        if (int w = readMap(map_width, map_height, map); w >= 0)
        {
            // game ended before round 0
            result.winner = w;
            result.reason = GameResult::ALL_TANKS_DEAD;
            result.rounds = 0;
            result.remaining_tanks = {
                static_cast<size_t>(playerTanksCount.at(1)),
                static_cast<size_t>(playerTanksCount.at(2))};
            std::unique_ptr<MySatelliteView> satelliteView = std::make_unique<MySatelliteView>(-1, tanks, shells, mines, walls);
            result.gameState = std::move(satelliteView);
            return result;
        }

        openVerboseFiles("GameManager", map_name, name1, name2);

        // step2: create tank algorithms
        for (const auto &pair : tanks)
        {
            Tank *tank = pair.second.get();
            std::unique_ptr<TankAlgorithm> algo;
            if (tank->getPlayerId() == 1)
            {
                algo = player1_tank_algo_factory(tank->getPlayerId(), tank->getTankId());
            }
            else
            {
                algo = player2_tank_algo_factory(tank->getPlayerId(), tank->getTankId());
            }

            tank->setTankAlgorithm(std::move(algo));
        }

        // step3: main loop
        int count = 0;

        while (true)
        {
            for (const auto &pair : tanks)
            {
                Tank *tank = pair.second.get();
                TankAlgorithm *algo = tank->getTankAlgorithm();
                ActionRequest move = algo->getAction();
                tank->setLastMove(move);
            }
            executeBattleInfoRequests(player1, player2);
            advanceShells();
            removeShells();
            advanceShells();
            removeObjectsFromTheBoard();
            executeTanksMoves(true);
            advanceShellsRecentlyFired();
            removeTanks();
            removeShells();

            executeTanksMoves(false);

            removeObjectsFromTheBoard();

            advanceShells();
            removeShells();
            advanceShells();
            removeObjectsFromTheBoard();

            outputTankMoves();
            gameStep++;
            printBoard();

            if (auto w = winnerByTanks())
            {
                result.winner = *w;
                result.reason = GameResult::ALL_TANKS_DEAD;
                break;
            }
            else if (gameStep >= maxSteps)
            {
                result.winner = 0;
                result.reason = GameResult::MAX_STEPS;
                break;
            }
            else if (totalShellsRemaining <= 0)
            {
                count++;
                if (count == MAX_STEPS_WITHOUT_SHELLS)
                {
                    result.winner = 0;
                    result.reason = GameResult::ZERO_SHELLS;
                    break;
                }
            }
        }

        // step4: declare results
        result.rounds = gameStep;
        result.remaining_tanks = {
            static_cast<size_t>(playerTanksCount.at(1)),
            static_cast<size_t>(playerTanksCount.at(2))};
        std::unique_ptr<MySatelliteView> satelliteView = std::make_unique<MySatelliteView>(-1, tanks, shells, mines, walls);
        result.gameState = std::move(satelliteView);

        if (verbose)
        {
            moves_out << "Summary: winner=" << result.winner << " reason=" << result.reason
                      << " total game steps=" << gameStep
                      << " player1 remaining tanks= " << result.remaining_tanks[0]
                      << " player2 remaining tanks= " << result.remaining_tanks[1] << std::endl;
        }

        clearGameState();
        return result;
    };

    void GameManager::printBoard()
    {
        if (!verbose)
            return;
        std::vector<std::vector<char>> board(height, std::vector<char>(width, '.'));
        std::pair<int, int> xy;

        for (const auto &pair : walls)
        {
            int x = pair.second.x / 2;
            int y = pair.second.y / 2;
            short health = pair.second.health;
            if (health == 2)
                board[y][x] = '#';
            else if (health == 1)
                board[y][x] = '/';
        }

        for (const auto &mine : mines)
        {
            xy = UC::inverseBijection(mine);
            int x = xy.first / 2;
            int y = xy.second / 2;
            board[y][x] = '@';
        }

        for (const auto &pair : shells)
        {
            Shell *a = pair.second.get();
            int x = a->getX() / 2;
            int y = a->getY() / 2;
            board[y][x] = '*';
        }

        for (const auto &pair : tanks)
        {
            Tank *tank = pair.second.get();
            int x = tank->getX() / 2;
            int y = tank->getY() / 2;
            char symbol = '0' + (tank->getPlayerId() % 10);
            board[y][x] = symbol;
        }

        viz_out << "\n=== Game Step " << gameStep << " ===\n";
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                viz_out << board[y][x];
            }
            viz_out << '\n';
        }
        viz_out << std::endl;
    }

    void GameManager::outputTankMoves()
    {
        if (!verbose)
            return;
        int wordSize = 0;
        for (int i = 0; i < totalTanks; i++)
        {
            wordSize = static_cast<int>(movesOfTanks[i].size());
            moves_out << movesOfTanks[i];
            if (wordSize > 0 && movesOfTanks[i][wordSize - 1] == ')' && wordSize >= 9 &&
                movesOfTanks[i].substr(wordSize - 9, 9) == " (killed)")
                movesOfTanks[i] = "killed";
            if (i != totalTanks - 1)
                moves_out << ", ";
        }
        moves_out << "\n";
    }

    void GameManager::openVerboseFiles(const std::string &gmName,
                                       const std::string &mapName,
                                       const std::string &alg1Name,
                                       const std::string &alg2Name)
    {
        if (!verbose)
            return;
        std::filesystem::create_directories(verbose_dir);
        std::filesystem::create_directories(visualization_dir);
        const auto base = make_unique_base(gmName, mapName, alg1Name, alg2Name);
        const auto moves_path = std::filesystem::path(verbose_dir) / (base + ".moves.txt");
        const auto viz_path = std::filesystem::path(visualization_dir) / (base + ".viz.txt");
        moves_out.open(moves_path, std::ios::out | std::ios::trunc);
        viz_out.open(viz_path, std::ios::out | std::ios::trunc);
    }

    void GameManager::clearGameState()
    {
        if (moves_out.is_open())
            moves_out.close();
        if (viz_out.is_open())
            viz_out.close();

        width = 0;
        height = 0;
        gameStep = 0;
        totalShellsRemaining = 0;
        maxSteps = 0;
        numShellsPerTank = 0;
        totalTanks = 0;

        movesOfTanks.clear();

        playerTanksCount.clear();
        playerTanksCount[1] = 0;
        playerTanksCount[2] = 0;

        tanks.clear();
        secondaryTanks.clear();

        shells.clear();
        secondaryShells.clear();
        shellsToRemove.clear();
        shellsFired.clear();

        mines.clear();

        walls.clear();
        wallsToRemove.clear();
    }
}
