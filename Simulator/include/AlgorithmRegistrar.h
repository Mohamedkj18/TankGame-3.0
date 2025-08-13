#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cassert>
#include "common/TankAlgorithm.h"
#include "common/Player.h"

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
    std::unordered_map<std::string,AlgorithmAndPlayerFactories> algorithms;
    static AlgorithmRegistrar registrar;

public:
    static AlgorithmRegistrar &getAlgorithmRegistrar();
    void createAlgorithmFactoryEntry(const std::string &name)
    {
        algorithms[name] = AlgorithmAndPlayerFactories(name);
    }
    void addPlayerFactoryToLastEntry(std::string so_name,PlayerFactory &&factory)
    {
        algorithms[so_name].setPlayerFactory(std::move(factory));
    }
    void addTankAlgorithmFactoryToLastEntry(std::string so_name,TankAlgorithmFactory &&factory)
    {
        algorithms[so_name].setTankAlgorithmFactory(std::move(factory));
    }
    struct BadRegistrationException
    {
        std::string name;
        bool hasName, hasPlayerFactory, hasTankAlgorithmFactory;
    };
    void validateLastRegistration(std::string so_name)
    {
        const auto &last = algorithms[so_name];
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
    
    auto begin() const
    {
        return algorithms.begin();
    }
    auto end() const
    {
        return algorithms.end();
    }
    std::size_t count() const { return algorithms.size(); }
    void clear() { algorithms.clear(); }
};