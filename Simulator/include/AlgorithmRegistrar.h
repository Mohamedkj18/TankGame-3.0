#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cassert>
#include "common/TankAlgorithm.h"
#include "common/Player.h"
#include <map>

class AlgorithmRegistrar
{
    class AlgorithmAndPlayerFactories
    {
        std::string so_name;
        TankAlgorithmFactory tankAlgorithmFactory;
        PlayerFactory playerFactory;

    public:
        AlgorithmAndPlayerFactories(const std::string &so_name) : so_name(so_name) {}
        void setTankAlgorithmFactory(TankAlgorithmFactory &&factory)
        {
            assert(tankAlgorithmFactory == nullptr);
            tankAlgorithmFactory = std::move(factory);
        }
        void setPlayerFactory(PlayerFactory &&factory)
        {
            assert(playerFactory == nullptr);
            playerFactory = std::move(factory);
        }
        const std::string &name() const { return so_name; }
        std::unique_ptr<Player> createPlayer(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells) const
        {
            return playerFactory(player_index, x, y, max_steps, num_shells);
        }
        std::unique_ptr<TankAlgorithm> createTankAlgorithm(int player_index, int tank_index) const
        {
            return tankAlgorithmFactory(player_index, tank_index);
        }
        bool hasPlayerFactory() const
        {
            return playerFactory != nullptr;
        }
        bool hasTankAlgorithmFactory() const
        {
            return tankAlgorithmFactory != nullptr;
        }
    };

    
    std::map<size_t,AlgorithmAndPlayerFactories> algorithms;
    static AlgorithmRegistrar registrar;
    static size_t algoID;
    

public:
    static AlgorithmRegistrar &getAlgorithmRegistrar();
    void createAlgorithmFactoryEntry(const std::string &name)
    {
        algorithms[algoID] = AlgorithmAndPlayerFactories(name);
    }
    void addPlayerFactoryToLastEntry(PlayerFactory &&factory)
    {
        algorithms[algoID].setPlayerFactory(std::move(factory));
    }
    void addTankAlgorithmFactoryToLastEntry(TankAlgorithmFactory &&factory)
    {
        algorithms[algoID++].setTankAlgorithmFactory(std::move(factory));
    }
    struct BadRegistrationException
    {
        std::string name;
        bool hasName, hasPlayerFactory, hasTankAlgorithmFactory;
    };
    void validateLastRegistration()
    {
        const auto &last = algorithms[algoID-1];
        bool hasName = (last.name() != "");
        if (!hasName || !last.hasPlayerFactory() || !last.hasTankAlgorithmFactory())
        {
            throw BadRegistrationException{
                last.name(),
                hasName,
                last.hasPlayerFactory(),
                last.hasTankAlgorithmFactory()};
        }
    }

    void initializeAlgoID();
    size_t getAlgoID();

    AlgorithmAndPlayerFactories &getPlayerAndAlgoFactory(size_t id)
    {
        if (algorithms.count(id) == 0) {
            throw std::runtime_error("Algorithm ID not found: " + std::to_string(id));
        }
        return algorithms[id];
    }

    auto begin() const
    {
        return algorithms.begin();
    }
    auto rbegin() const
    {
        return algorithms.rbegin();
    }
    auto end() const
    {
        return algorithms.end();
    }
    std::size_t count() const { return algorithms.size(); }
    void clear() { algorithms.clear(); }
};