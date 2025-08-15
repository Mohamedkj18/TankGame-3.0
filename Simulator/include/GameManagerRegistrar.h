#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cassert>
#include <map>
#include "common/AbstractGameManager.h"

class GameManagerRegistrar
{
public:
    class GameManagerFactory
    {
        std::string so_name;
        std::function<std::unique_ptr<AbstractGameManager>(bool verbose)> factory;

    public:
        GameManagerFactory(const std::string &so_name) : so_name(so_name) {}
        void setFactory(std::function<std::unique_ptr<AbstractGameManager>(bool verbose)> &&factory)
        {
            assert(this->factory == nullptr);
            this->factory = std::move(factory);
        }
        const std::string &name() const { return so_name; }
        std::unique_ptr<AbstractGameManager> create(bool verbose) const
        {
            return factory(verbose);
        }
        bool hasFactory() const
        {
            return factory != nullptr;
        }
    };
    std::map<std::string,GameManagerFactory> gameManagers;
    static GameManagerRegistrar registrar;

public:
    static GameManagerRegistrar &getGameManagerRegistrar()
    {
        return registrar;
    }
    void createGameManagerFactoryEntry(const std::string &name)
    {
        gameManagers[name] = GameManagerFactory(name);
    }
    void addGameManagerFactoryToLastEntry(std::string so_name, std::function<std::unique_ptr<AbstractGameManager>(bool verbose)> &&factory)
    {
        gameManagers[so_name].setFactory(std::move(factory));
    }
    struct BadRegistrationException
    {
        std::string name;
        bool hasName, hasFactory;
    };
    void validateLastRegistration(std::string so_name)
    {
        const auto &last = gameManagers[so_name];
        bool hasName = (last.name() != "");
        if (!hasName || !last.hasFactory())
        {
            throw BadRegistrationException{
                last.name(),
                hasName,
                last.hasFactory()};
        }
    }
    void removeLast(std::string so_name)
    {
        if (gameManagers.empty())
        {
            throw std::runtime_error("No game manager registrations to remove.");
        }
        gameManagers.erase(so_name);
    }
    auto begin() const
    {
        return gameManagers.begin();
    }
    auto rbegin() const
    {
        return gameManagers.rbegin();
    }
    auto end() const
    {        return gameManagers.end();
    }
};