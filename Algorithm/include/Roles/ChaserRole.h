#pragma once

#include "Role.h"

namespace Algorithm_212788293_212497127
{
    class TankAlgorithm_212788293_212497127;

    class ChaserRole : public Role
    {
    public:
        using Role::Role;
        ~ChaserRole() override = default;

        std::vector<std::pair<int, int>> prepareActions(TankAlgorithm_212788293_212497127 &algo) override;

        std::string getRoleName() const override { return "Chaser"; }
        std::unique_ptr<Role> clone() const override
        {
            return std::make_unique<ChaserRole>(*this);
        }
    };
}