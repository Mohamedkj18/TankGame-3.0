#ifndef ALG_212788293_212497127_TANKALGO_H
#define ALG_212788293_212497127_TANKALGO_H

#include "common/TankAlgorithm.h"
#include "common/ActionRequest.h"
#include "BattleInfoLite.h"

namespace Algorithm_212788293_212497127
{

  class TankAlgorithm_212788293_212497127 : public TankAlgorithm
  {
  public:
    TankAlgorithm_212788293_212497127(int player_index, int tank_index) noexcept;

    void updateBattleInfo(::BattleInfo &info) override;
    ::ActionRequest getAction() override;

  private:
    static int dirIndexFromDelta(int dx, int dy); // (dx,dy) -> 0..7
    static ::ActionRequest rotateStepToward(int fromIdx, int toIdx);

  private:
    int player_idx_{0};
    int tank_idx_{0};

    // latest order from Player
    BattleInfoLite bi_{};
    bool have_bi_{false};

    // our own facing (0..7), tracked deterministically by our own commands
    int facing_idx_{0};

    // script cursor
    std::uint8_t plan_idx_{0};
  };

} // namespace Algorithm_212788293_212497127

#endif
