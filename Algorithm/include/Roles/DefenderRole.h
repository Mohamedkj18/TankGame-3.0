#pragma once

#include "Role.h"
#include <optional>

namespace Algorithm_212788293_212497127
{
    class TankAlgorithm_212788293_212497127;

    class DefenderRole : public Role
    {

    public:
        using Role::Role;
        ~DefenderRole() override = default;

        std::vector<std::pair<int, int>> prepareActions(TankAlgorithm_212788293_212497127 &algo) override;
        std::string getRoleName() const override { return "Defender"; }
        std::unique_ptr<Role> clone() const override
        {
            return std::make_unique<DefenderRole>(*this);
        }
        std::vector<ActionRequest> getNextMoves(std::vector<std::pair<int, int>> path, std::pair<int, int> target, TankAlgorithm_212788293_212497127 &algo);
    };
}