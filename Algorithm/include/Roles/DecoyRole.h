#pragma once
#include "Role.h"

namespace Algorithm_212788293_212497127
{
    class TankAlgorithm_212788293_212497127;

    class DecoyRole : public Role
    {
    public:
        using Role::Role;
        ~DecoyRole() override = default;

        std::vector<std::pair<int, int>> prepareActions(TankAlgorithm_212788293_212497127 &algo) override;
        std::string getRoleName() const override { return "Decoy"; }
        std::unique_ptr<Role> clone() const override
        {
            return std::make_unique<DecoyRole>(*this);
        }
        std::vector<ActionRequest> getNextMoves(std::vector<std::pair<int, int>> path, TankAlgorithm_212788293_212497127 &algo);
    };
}