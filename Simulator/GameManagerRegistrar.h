#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cassert>
#include "common/AbstractGameManager.h"


class GameManagerRegistrar {
public:
    class GameManagerFactory{
        std::string so_name;
        std::function<std::unique_ptr<AbstractGameManager>(bool verbose)> factory;
    public:
        GameManagerFactory(const std::string& so_name) : so_name(so_name) {}
        void setFactory(std::function<std::unique_ptr<AbstractGameManager>(bool verbose)>&& factory) {
            assert(this->factory == nullptr);
            this->factory = std::move(factory);
        }
        const std::string& name() const { return so_name; }
        std::unique_ptr<AbstractGameManager> create(bool verbose) const {
            return factory(verbose);
        }
    };
    std::vector<GameManagerFactory> gameManagers;
    static GameManagerRegistrar registrar;
public:
    static GameManagerRegistrar& getGameManagerRegistrar() {
        return registrar;
    }
    void createGameManagerFactoryEntry(const std::string& name) {
        gameManagers.emplace_back(name);
    }
    void addGameManagerFactoryToLastEntry(std::function<std::unique_ptr<AbstractGameManager>(bool verbose)>&& factory) {
        gameManagers.back().setFactory(std::move(factory));
    }
    struct BadRegistrationException {
        std::string name;
        bool hasName, hasFactory;
    };
    void validateLastRegistration() {
        const auto& last = gameManagers.back();
        bool hasName = (last.name() != "");
        if(!hasName || !last.factory) {
            throw BadRegistrationException{
                .name = last.name(),
                .hasName = hasName,
                .hasFactory = last.factory != nullptr
            };
        }
    }
    void removeLast() {
        if(gameManagers.empty()) {
            throw std::runtime_error("No game manager registrations to remove.");
        }
        gameManagers.pop_back();
    }
};