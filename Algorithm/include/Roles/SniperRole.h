#pragma once
#include "Role.h"

namespace Algorithm_212788293_212497127
{
    class TankAlgorithm_212788293_212497127;

    class SniperRole : public Role
    {

    public:
        using Role::Role;
        ~SniperRole() override = default;

        std::vector<std::pair<int, int>> prepareActions(TankAlgorithm_212788293_212497127 &algo) override;

        std::string getRoleName() const override { return "Sniper"; }
        std::unique_ptr<Role> clone() const override
        {
            return std::make_unique<SniperRole>(*this);
        }
    };
}