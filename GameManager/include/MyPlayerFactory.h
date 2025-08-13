#include <cstddef>
#include <memory>
#include "Player.h"

class MyPlayerFactory : public PlayerFactory
{
public:
    std::unique_ptr<Player> create(int player_index, size_t x, size_t y,
                                   size_t max_steps, size_t num_shells) const override;
};
