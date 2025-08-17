#pragma once

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
    std::map<size_t,GameManagerFactory> gameManagers;
    static GameManagerRegistrar registrar;
    static size_t GameManagerCount;

public:
    static GameManagerRegistrar &getGameManagerRegistrar()
    {
        return registrar;
    }
    static void initializeGameManagerCount()
    {
        GameManagerCount = 0;
    }
    size_t getGameManagerCount() const
    {
        return GameManagerCount;
    }

    GameManagerFactory &getGameManagerFactory(size_t id)
    {
        auto it = gameManagers.find(id);
        if (it == gameManagers.end())
        {
            throw std::runtime_error("GameManager not found: " + std::to_string(id));
        }
        return it->second;
    }

    void createGameManagerFactoryEntry(const std::string &name)
    {
        gameManagers[GameManagerCount] = GameManagerFactory(name);
    }
    void addGameManagerFactoryToLastEntry(std::function<std::unique_ptr<AbstractGameManager>(bool verbose)> &&factory)
    {
        gameManagers[GameManagerCount++].setFactory(std::move(factory));
    }
    struct BadRegistrationException
    {
        std::string name;
        bool hasName, hasFactory;
    };
    void validateLastRegistration()
    {
        const auto &last = gameManagers[GameManagerCount - 1];
        bool hasName = (last.name() != "");
        if (!hasName || !last.hasFactory())
        {
            throw BadRegistrationException{
                last.name(),
                hasName,
                last.hasFactory()};
        }
    }
    void removeLast()
    {
        if (gameManagers.empty())
        {
            throw std::runtime_error("No game manager registrations to remove.");
        }
        gameManagers.erase(GameManagerCount - 1);
        --GameManagerCount;
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