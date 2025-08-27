#pragma once
#include <vector>
#include <string>
#include <cassert>

#include "common/SatelliteView.h"
#include "common/TankAlgorithm.h"
#include "BattleInfoLite.h"

namespace Algorithm_212788293_212497127
{

    // Grid-backed SatelliteView (chars like '#','@','*','%','1','2','.')
    struct FakeSatelliteView : public ::SatelliteView
    {
        std::size_t W{}, H{};
        std::vector<std::string> rows;

        FakeSatelliteView(std::vector<std::string> r)
            : W(r.empty() ? 0 : r[0].size()), H(r.size()), rows(std::move(r)) {}

        char getObjectAt(std::size_t x, std::size_t y) const override
        {
            assert(y < H && x < W);
            return rows[y][x];
        }
    };

    // Minimal Tank that records the last BattleInfoLite it received
    struct FakeTankAlgorithm : public ::TankAlgorithm
    {
        int player_index, tank_index;
        // Captured fields
        RoleTag last_tag{RoleTag::Aggressor};
        Cell last_waypoint{0, 0};
        bool got_update{false};

        FakeTankAlgorithm(int p, int t) : player_index(p), tank_index(t) {}

        void updateBattleInfo(::BattleInfo &base) override
        {
            if (auto *bi = dynamic_cast<BattleInfoLite *>(&base))
            {
                last_tag = bi->tag;
                last_waypoint = bi->waypoint;
                got_update = true;
            }
            else
            {
                assert(false && "Expected BattleInfoLite in updateBattleInfo");
            }
        }
        ::ActionRequest getAction() override
        {
            // Dummy action; not used in tests
            return ::ActionRequest::DoNothing;
        }
        // If your ::TankAlgorithm base has other pure-virtuals,
        // add empty overrides here to satisfy the linker.
        // e.g., virtual ::ActionRequest requestAction() override { return ::ActionRequest::DoNothing; }
    };

} // namespace Algorithm_212788293_212497127
