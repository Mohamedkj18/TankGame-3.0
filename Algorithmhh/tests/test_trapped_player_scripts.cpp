#include <cassert>
#include <iostream>
#include "fake_satellite.h"

#include "Player_212788293_212497127.h"
#include "BattleInfoLite.h"
#include "common/TankAlgorithm.h"

using namespace Algorithm_212788293_212497127;

struct MockTank : public ::TankAlgorithm
{
    BattleInfoLite last{};
    void updateBattleInfo(::BattleInfo &info) override
    {
        auto *p = dynamic_cast<BattleInfoLite *>(&info);
        assert(p != nullptr);
        last = *p;
    }
    ::ActionRequest getAction() override { return ::ActionRequest::DoNothing; }
};

int main()
{
    // ---------- Surrounded by WALLS -> breacher (shoot) ----------
    {
        const std::size_t W = 5, H = 5;
        FakeSatelliteView sat(W, H);
        sat.clear('.');
        const std::size_t tx = 2, ty = 2;
        // ring of walls
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                if (!(dx == 0 && dy == 0))
                {
                    sat.set((tx + dx + W) % W, (ty + dy + H) % H, '#');
                }
        // our tank
        sat.set(tx, ty, '1');

        Player_212788293_212497127 player(1, W, H, 200, 10);
        MockTank mt;
        sat.setRequestingTank(tx, ty);
        player.updateTankWithBattleInfo(mt, sat);

        // Expect: plan_len==1 and plan_shoot[0]==1 (breach)
        assert(mt.last.plan_len == 1);
        assert(mt.last.plan_shoot[0] == 1);
        std::cout << "trapped-walls: plan_len=1, shoot=1 OK\n";
    }

    // ---------- Surrounded by MINES -> sniper rules ----------
    {
        const std::size_t W = 7, H = 5;
        FakeSatelliteView sat(W, H);
        sat.clear('.');
        const std::size_t tx = 3, ty = 2;

        // ring of mines
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                if (!(dx == 0 && dy == 0))
                {
                    sat.set((tx + dx + W) % W, (ty + dy + H) % H, '@');
                }

        // enemy north; mine at (3,1) blocks LOS (already placed by ring)
        sat.set(3, 0, '2');
        sat.set(tx, ty, '1');

        Player_212788293_212497127 player(1, W, H, 200, 10);
        MockTank mt;
        sat.setRequestingTank(tx, ty);
        player.updateTankWithBattleInfo(mt, sat);

        // Two acceptable outcomes by our policy:
        // 1) Clear enemy ray exists -> plan_len==1 with shoot flag
        // 2) Ray blocked by mine -> no script to move (stay), single-step scan fields set (shoot_when_aligned=false)
        bool ok_case1 = (mt.last.plan_len == 1 && mt.last.plan_shoot[0] == 1);
        bool ok_case2 = (mt.last.plan_len == 0 && mt.last.shoot_when_aligned == false);
        assert(ok_case1 || ok_case2);
        std::cout << "trapped-mines: script or scan OK\n";
    }

    return 0;
}
